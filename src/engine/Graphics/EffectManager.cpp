#include "EffectManager.h"
#include "TextureManager.h"
#include "Object3dCommon.h"

// ============================================
// RingEffect
// ============================================
RingEffect::RingEffect(const Vector3& pos, const Vector4& color, float lifeTime, float maxRadius) {
	position_ = pos;
	color_ = color;
	lifeTime_ = lifeTime;
	currentTime_ = 0.0f;
	maxRadius_ = maxRadius;
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/white1x1.png");
}

void RingEffect::Update() {
	currentTime_ += 1.0f / 60.0f; // 約60FPSを想定
	if (currentTime_ >= lifeTime_) {
		isDead_ = true;
	}

	float t = currentTime_ / lifeTime_;
	// イージング（徐々に薄くなる、徐々に広がる）
	currentRadius_ = maxRadius_ * (1.0f - std::powf(1.0f - t, 3.0f)); 
	color_.w = (1.0f - t); // アルファ減衰

	scale_ = { currentRadius_, currentRadius_, currentRadius_ };
}

void RingEffect::Draw(Camera* camera) {
	if (isDead_) return;
	// X軸方向に90度回転(床に平行にする)
	Vector3 rot = { 3.141592f / 2.0f, 0.0f, 0.0f };
	PrimitiveModel::GetInstance()->DrawRing(scale_, rot, position_, color_, textureIndex_, camera, BlendMode::kAdd);
}

// ============================================
// CylinderEffect
// ============================================
CylinderEffect::CylinderEffect(const Vector3& pos, const Vector4& color, float lifeTime, float maxScaleY, float maxScaleX) {
	position_ = pos;
	color_ = color;
	lifeTime_ = lifeTime;
	currentTime_ = 0.0f;
	maxScaleY_ = maxScaleY;
	maxScaleX_ = maxScaleX;
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/white1x1.png");
}

void CylinderEffect::Update() {
	currentTime_ += 1.0f / 60.0f;
	if (currentTime_ >= lifeTime_) {
		isDead_ = true;
	}

	float t = currentTime_ / lifeTime_;
	// 高さは徐々に伸び、幅は細くなる
	scale_.x = maxScaleX_ * (1.0f - t);
	scale_.z = maxScaleX_ * (1.0f - t);
	scale_.y = maxScaleY_ * std::powf(t, 0.5f);
	color_.w = (1.0f - t); // アルファ減衰
}

void CylinderEffect::Draw(Camera* camera) {
	if (isDead_) return;
	PrimitiveModel::GetInstance()->DrawCylinder(scale_, {0,0,0}, position_, color_, textureIndex_, camera, BlendMode::kAdd);
}

// ============================================
// HitRotPlaneEffect
// ============================================
HitRotPlaneEffect::HitRotPlaneEffect(const Vector3& pos, const Vector4& color, float lifeTime, float size) {
	position_ = pos;
	color_ = color;
	lifeTime_ = lifeTime;
	currentTime_ = 0.0f;
	targetSize_ = size;
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/circle2.png");

	// ランダムな初期角度と回転速度の決定
	rotationEulers_ = { (float)(rand() % 314) / 100.0f, (float)(rand() % 314) / 100.0f, (float)(rand() % 314) / 100.0f };
	// 回転速度もランダムに（少し遅めにして斬撃っぽくする）
	rotationSpeed_ = { 
		((float)(rand() % 100) / 1000.0f) - 0.05f, 
		((float)(rand() % 100) / 1000.0f) - 0.05f, 
		((float)(rand() % 100) / 1000.0f) - 0.05f 
	};
}

void HitRotPlaneEffect::Update() {
	currentTime_ += 1.0f / 60.0f;
	if (currentTime_ >= lifeTime_) {
		isDead_ = true;
	}

	float t = currentTime_ / lifeTime_;
	
	rotationEulers_.x += rotationSpeed_.x;
	rotationEulers_.y += rotationSpeed_.y;
	rotationEulers_.z += rotationSpeed_.z;

	// 縦長にするため、XとZを細くし、Yを長くする
	scale_.x = targetSize_ * 0.15f * (1.0f - t);
	scale_.y = targetSize_ * 1.5f * (1.0f - t);
	scale_.z = targetSize_ * 0.15f * (1.0f - t);
	color_.w = (1.0f - t);
}

void HitRotPlaneEffect::Draw(Camera* camera) {
	if (isDead_) return;
	PrimitiveModel::GetInstance()->DrawPlane(scale_, rotationEulers_, position_, color_, textureIndex_, camera, BlendMode::kAdd);
}

// ============================================
// BillboardParticleEffect
// ============================================
BillboardParticleEffect::BillboardParticleEffect(const Vector3& pos, const std::string& groupName, uint32_t count) {
	position_ = pos;
	lifeTime_ = 0.1f; // 発行するだけなので短く
	ParticleManager::GetInstance()->Emit(groupName, position_, count);
}
void BillboardParticleEffect::Update() {
	currentTime_ += 1.0f / 60.0f;
	if (currentTime_ >= lifeTime_) isDead_ = true;
}
void BillboardParticleEffect::Draw(Camera* camera) {
	// 描画自体はParticleManagerが担当するため何もしない
	(void)camera;
}

// ============================================
// 爆発パーティクルエフェクト（再構築：もくもく感＋スピード感）
// ============================================
ExplosionParticleEffect::ExplosionParticleEffect(const Vector3& pos) {
	position_ = pos;
	lifeTime_ = 2.0f; 
	currentTime_ = 0.0f;

	ParticleManager* pm = ParticleManager::GetInstance();

	// --- 1. Flash (閃光): 超高速で最大化してすぐ消える ---
	flashScale_ = 30.0f; 
	flashAlpha_ = 1.0f;
	
	// --- 2. Ground Shockwave Dust (地面を這う土煙の波動) ---
	// 波動も数を多く、間隔を狭めるため大量のパーティクルを円周状に放出
	for (int i = 0; i < 180; i++) { // 数を大幅に増加
		float angle = ((float)(rand() % 360)) * 3.141592f / 180.0f;
		Vector3 dir = { cosf(angle), 0.0f, sinf(angle) };
		
		ParticleParameters waveParams;
		float speed = 15.0f + (rand() % 100) / 10.0f; // 速度を落としてゆっくり広がるように
		Vector3 baseVel = { dir.x * speed, 0.5f, dir.z * speed };
		Vector3 spread = { std::abs(dir.x * 2.0f), 0.5f, std::abs(dir.z * 2.0f) };
		
		waveParams.minVelocity = { baseVel.x - spread.x, baseVel.y - spread.y, baseVel.z - spread.z };
		waveParams.maxVelocity = { baseVel.x + spread.x, baseVel.y + spread.y, baseVel.z + spread.z };
		
		// 砂埃のような色味
		float shade = 0.5f + ((rand() % 100) / 100.0f) * 0.3f;
		waveParams.minColor = { shade, shade * 0.9f, shade * 0.8f, 0.02f }; 
		waveParams.maxColor = { shade + 0.1f, (shade + 0.1f) * 0.9f, (shade + 0.1f) * 0.8f, 0.08f };
		waveParams.minLifeTime = 1.5f;
		waveParams.maxLifeTime = 2.5f;
		waveParams.acceleration = { 0.0f, -0.2f, 0.0f }; 
		waveParams.minScale = 6.0f;
		waveParams.maxScale = 12.0f;
		waveParams.endScale = 30.0f;
		waveParams.fadeOut = true;
		waveParams.scaleEasing = 0.5f; 
		
		waveParams.minRotation = 0.0f;
		waveParams.maxRotation = 6.28f;
		waveParams.minRotationSpeed = -2.0f;
		waveParams.maxRotationSpeed = 2.0f;

		pm->Emit("ExplosionSmoke", position_, waveParams, 4); // 重ねて表示
	}
	
	lightIntensity_ = 300.0f; // 強烈な光

	// --- 3. Core Fire (爆発の核となる炎): 広がりながら消える ---
	for (int i = 0; i < 40; i++) {
		// 球状に広がるランダムベクトル
		Vector3 offset = { ((rand() % 100) / 50.0f - 1.0f),
		                   ((rand() % 100) / 50.0f - 1.0f),
		                   ((rand() % 100) / 50.0f - 1.0f) };
		offset = Normalize(offset);

		ParticleParameters fireParams;
		// 外側へ向かう初速
		float speed = 10.0f + (rand() % 200) / 10.0f;
		Vector3 baseVel = { offset.x * speed, offset.y * speed, offset.z * speed };
		Vector3 spread = { std::abs(baseVel.x * 0.2f), std::abs(baseVel.y * 0.2f), std::abs(baseVel.z * 0.2f) };
		fireParams.minVelocity = { baseVel.x - spread.x, baseVel.y - spread.y, baseVel.z - spread.z };
		fireParams.maxVelocity = { baseVel.x + spread.x, baseVel.y + spread.y, baseVel.z + spread.z };

		fireParams.minColor = { 1.0f, 0.4f, 0.0f, 1.0f }; // 濃いオレンジ
		fireParams.maxColor = { 1.0f, 0.8f, 0.2f, 1.0f }; // 黄色っぽいオレンジ
		fireParams.minLifeTime = 0.2f;
		fireParams.maxLifeTime = 0.6f; 
		fireParams.acceleration = { 0.0f, 0.0f, 0.0f };   
		fireParams.minScale = 5.0f;
		fireParams.maxScale = 12.0f;
		fireParams.endScale = 0.0f; // 急速に縮んで消える
		fireParams.fadeOut = true;
		fireParams.scaleEasing = 0.2f;

		// 回転
		fireParams.minRotation = 0.0f;
		fireParams.maxRotation = 6.28f;
		fireParams.minRotationSpeed = -0.5f;
		fireParams.maxRotationSpeed = 0.5f;

		pm->Emit("ExplosionFire", position_, fireParams, 1);
	}

	// --- 4. Core (煙): 厚みのある不透明な雲（もくもく感） ---
	// 輪郭が見えないよう、透明度をかなり落として枚数を大幅に増やし、柔らかく重ねる
	for (int i = 0; i < 100; i++) { // 数をさらに増加
		// 発生位置を下に下げる（Y軸のオフセットを低く設定）
		Vector3 offset = { ((rand() % 100) / 50.0f - 1.0f) * 3.5f,
		                   ((rand() % 100) / 50.0f - 0.5f) * 2.0f, // -1.0〜+3.0付近に発生
		                   ((rand() % 100) / 50.0f - 1.0f) * 3.5f };

		ParticleParameters smokeParams;
		// 煙全体は極めて遅くしつつ、横方向（X, Z）にだけジワジワと広がるように設定
		Vector3 baseVel = { offset.x * 0.15f, offset.y * 0.02f + 0.05f, offset.z * 0.15f };
		Vector3 sSpread = { std::abs(offset.x * 0.05f) + 0.05f, 0.1f, std::abs(offset.z * 0.05f) + 0.05f };
		smokeParams.minVelocity = { baseVel.x - sSpread.x, baseVel.y - sSpread.y, baseVel.z - sSpread.z };
		smokeParams.maxVelocity = { baseVel.x + sSpread.x, baseVel.y + sSpread.y, baseVel.z + sSpread.z };
		
		// 輪郭を目立たなくするため、Alphaを極限まで落とす（0.015〜0.08）
		float shade = 0.1f + ((rand() % 100) / 100.0f) * 0.2f; 
		smokeParams.minColor = { shade, shade, shade, 0.015f };  
		smokeParams.maxColor = { shade + 0.1f, shade + 0.1f, shade + 0.1f, 0.08f };     
		// ライフタイムを若干長く
		smokeParams.minLifeTime = 2.0f;
		smokeParams.maxLifeTime = 3.5f;
		smokeParams.acceleration = { 0.0f, 0.1f, 0.0f };  // 上昇速度を極端に遅くする
		
		// サイズをさらに大きくして重なりを増やす
		smokeParams.minScale = 16.0f;
		smokeParams.maxScale = 24.0f;
		smokeParams.endScale = 50.0f; 
		smokeParams.fadeOut = true;
		smokeParams.scaleEasing = 0.6f;  
		
		// ランダムな初期回転と、ゆっくりとした回転を与える
		smokeParams.minRotation = 0.0f;
		smokeParams.maxRotation = 6.28f; // 2PI
		smokeParams.minRotationSpeed = -0.5f;
		smokeParams.maxRotationSpeed = 0.5f;

		pm->Emit("ExplosionSmoke", Add(position_, offset), smokeParams, 5); 
	}

	// --- 5. Debris (破片): 四方八方に飛び散る ---
	int debrisCount = 10 + (rand() % 5); 
	for (int i = 0; i < debrisCount; i++) {
		Debris d;
		d.pos = position_;
		// 半球状に広がるランダムベクトル
		Vector3 dir = { ((rand() % 100) / 50.0f - 1.0f),
		                ((rand() % 100) / 100.0f) + 0.2f, // 上方向中心
		                ((rand() % 100) / 50.0f - 1.0f) };
		dir = Normalize(dir);
		float dSpeed = 20.0f + (rand() % 300) / 10.0f;
		d.vel = Multiply(dSpeed, dir);
		debris_.push_back(d);
	}
}

void ExplosionParticleEffect::Update() {
	float dt = 1.0f / 60.0f;
	currentTime_ += dt;
	if (currentTime_ >= lifeTime_) {
		isDead_ = true;
		return;
	}

	// --- Flash (閃光): 2〜3フレームで一瞬で消える ---
	if (flashAlpha_ > 0.0f) {
		flashScale_ += 80.0f * dt;
		flashAlpha_ -= 8.0f * dt; 
		if (flashAlpha_ < 0.0f) flashAlpha_ = 0.0f;
	}

	// --- Light (環境光) ---
	if (lightIntensity_ > 0.0f) {
		lightIntensity_ -= 600.0f * dt; 
		if (lightIntensity_ < 0.0f) lightIntensity_ = 0.0f;
	}

	// --- Debris & Trails (破片と煙の尾) ---
	ParticleManager* pm = ParticleManager::GetInstance();
	for (auto& d : debris_) {
		if (d.pos.y < -10.0f) continue; 

		d.pos = Add(d.pos, Multiply(dt, d.vel));
		d.vel.y -= 9.81f * 4.0f * dt; // 非常に強い重力で急落下する

		// 破片の軌跡：その場に留まり、膨張しながら消える（数珠つなぎにならないよう密集させる）
		ParticleParameters trailParams;
		trailParams.minVelocity = { -0.5f, -0.5f, -0.5f }; // ほとんど動かない
		trailParams.maxVelocity = {  0.5f,  0.5f,  0.5f };
		trailParams.minColor = { 0.1f, 0.1f, 0.1f, 0.05f }; // 非常に薄く重ねる
		trailParams.maxColor = { 0.2f, 0.2f, 0.2f, 0.15f };
		trailParams.minLifeTime = 0.5f;
		trailParams.maxLifeTime = 1.0f;
		trailParams.minScale = 4.0f;
		trailParams.maxScale = 6.0f;
		trailParams.endScale = 12.0f;
		trailParams.fadeOut = true;
		trailParams.scaleEasing = 0.5f;

		trailParams.minRotation = 0.0f;
		trailParams.maxRotation = 6.28f;
		trailParams.minRotationSpeed = -1.0f;
		trailParams.maxRotationSpeed = 1.0f;

		pm->Emit("ExplosionSmoke", d.pos, trailParams, 2);
	}
}

void ExplosionParticleEffect::Draw(Camera* camera) {
	PrimitiveModel* prim = PrimitiveModel::GetInstance();

	// --- Flash (閃光) ---
	// のっぺりした四角ではなく、circle2.pngで柔らかく強烈な発光
	if (flashAlpha_ > 0.0f) {
		uint32_t flashTex = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/circle2.png");
		Vector4 flashColor = { 1.0f, 0.8f, 0.3f, flashAlpha_ };
		Vector3 scale = { flashScale_, flashScale_, flashScale_ };
		
		Vector3 rotate = camera->GetRotate(); // カメラ目線
		prim->DrawPlane(scale, rotate, position_, flashColor, flashTex, camera, BlendMode::kAdd);
	}

	// --- Dynamic Light ---
	if (lightIntensity_ > 0.0f) {
		PointLight* pLight = Object3dCommon::GetInstance()->GetPointLightData();
		if (pLight) {
			pLight->position = position_;
			pLight->color = { 1.0f, 0.5f, 0.1f, 1.0f }; 
			pLight->intensity = lightIntensity_;
			pLight->radius = 400.0f; 
			pLight->decay = 2.5f;
		}
	}
}

// ============================================
// MuzzleFlashEffect (発砲時の閃光 + 火花)
// ============================================
MuzzleFlashEffect::MuzzleFlashEffect(const Vector3& pos, const Vector3& direction) {
	position_ = pos;
	direction_ = Normalize(direction);
	lifeTime_ = 0.1f; // 発行するだけなので短く

	ParticleManager* pm = ParticleManager::GetInstance();

	// === 1. 火花 (Sparks): 小さく多数を射撃方向に沿って飛散 ===
	{
		ParticleParameters sparkParams;
		float sparkBase = 6.0f;
		float sparkSpread = 4.0f;
		sparkParams.minVelocity = {
			direction_.x * sparkBase - sparkSpread,
			direction_.y * sparkBase - sparkSpread,
			direction_.z * sparkBase - sparkSpread
		};
		sparkParams.maxVelocity = {
			direction_.x * sparkBase + sparkSpread,
			direction_.y * sparkBase + sparkSpread,
			direction_.z * sparkBase + sparkSpread
		};
		sparkParams.minColor = { 1.0f, 0.6f, 0.1f, 1.0f };  // オレンジ色
		sparkParams.maxColor = { 1.0f, 0.8f, 0.2f, 1.0f };   // 少し明るいオレンジ
		sparkParams.minLifeTime = 0.03f;
		sparkParams.maxLifeTime = 0.12f;
		sparkParams.acceleration = { 0.0f, -0.02f, 0.0f };
		sparkParams.minScale = 0.02f; // 小さく
		sparkParams.maxScale = 0.08f;
		sparkParams.endScale = 0.0f;
		sparkParams.fadeOut = true;
		sparkParams.scaleEasing = 0.5f;

		// ストレッチ（引き伸ばし）ビルボードを有効化
		sparkParams.isStretched = true;
		sparkParams.stretchFactor = 0.08f; // 速度に応じて長さが変わる係数

		pm->Emit("ExplosionSpark", position_, sparkParams, 60); // 30 -> 60 に増やす
	}

	// === 2. フラッシュコア: 銃口で一瞬大きく光る閃光 ===
	{
		ParticleParameters flashParams;
		flashParams.minVelocity = { 0.0f, 0.0f, 0.0f };
		flashParams.maxVelocity = { 0.0f, 0.0f, 0.0f };
		flashParams.minColor = { 1.0f, 0.5f, 0.05f, 1.0f }; // 濃いオレンジ
		flashParams.maxColor = { 1.0f, 0.7f, 0.1f, 1.0f };  // オレンジ
		flashParams.minLifeTime = 0.03f;
		flashParams.maxLifeTime = 0.06f;
		flashParams.acceleration = { 0.0f, 0.0f, 0.0f };
		flashParams.minScale = 1.0f; // 3.0 -> 1.0 に小さく
		flashParams.maxScale = 2.0f; // 5.0 -> 2.0 に小さく
		flashParams.endScale = 0.0f;
		flashParams.fadeOut = true;
		flashParams.scaleEasing = 0.2f;
		pm->Emit("ExplosionFire", position_, flashParams, 10); // 4 -> 10 に増やす
	}

	// === 3. スモーク (Smoke): 発砲後にほんの一瞬残る微量の煙 ===
	{
		ParticleParameters smokeParams;
		smokeParams.minVelocity = { -2.0f, -2.0f, -2.0f };
		smokeParams.maxVelocity = { 2.0f, 2.0f, 2.0f };
		// 進行方向（逆向き）に少し流れる
		smokeParams.minVelocity.x -= direction_.x * 3.0f;
		smokeParams.minVelocity.y -= direction_.y * 3.0f;
		smokeParams.minVelocity.z -= direction_.z * 3.0f;
		smokeParams.maxVelocity.x -= direction_.x * 3.0f;
		smokeParams.maxVelocity.y -= direction_.y * 3.0f;
		smokeParams.maxVelocity.z -= direction_.z * 3.0f;
		
		// 境目を消すため、1枚あたりを極端に薄く（Alpha: 0.02 ~ 0.08）する
		smokeParams.minColor = { 0.4f, 0.4f, 0.4f, 0.02f }; 
		smokeParams.maxColor = { 0.6f, 0.6f, 0.6f, 0.08f };
		smokeParams.minLifeTime = 0.3f;
		smokeParams.maxLifeTime = 0.6f;
		smokeParams.acceleration = { 0.0f, 1.0f, 0.0f }; // 少し上に昇る
		
		// サイズを大きくし、ふんわりと広げる
		smokeParams.minScale = 2.0f;
		smokeParams.maxScale = 4.0f;
		smokeParams.endScale = 8.0f; 
		smokeParams.fadeOut = true;
		smokeParams.scaleEasing = 0.3f; // 最初は一気に広がり、後半はゆっくり
		
		// 量を大幅に増やして重ねることで、境界のない滑らかな雲を作る
		pm->Emit("ExplosionSmoke", position_, smokeParams, 40);
	}
}

void MuzzleFlashEffect::Update() {
	currentTime_ += 1.0f / 60.0f;
	if (currentTime_ >= lifeTime_) isDead_ = true;
}

void MuzzleFlashEffect::Draw(Camera* camera) {
	// 描画自体はParticleManagerが担当するため何もしない
	(void)camera;
}

// ============================================
// EffectManager
// ============================================
std::unique_ptr<EffectManager> EffectManager::instance_ = nullptr;

EffectManager* EffectManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_.reset(new EffectManager());
	}
	return instance_.get();
}

void EffectManager::Initialize() {
	effects_.clear();
}

void EffectManager::Update() {
	for (auto it = effects_.begin(); it != effects_.end();) {
		(*it)->Update();
		if ((*it)->IsDead()) {
			it = effects_.erase(it);
		}
		else {
			++it;
		}
	}
}

void EffectManager::Draw(Camera* camera) {
	for (auto& effect : effects_) {
		effect->Draw(camera);
	}
}

void EffectManager::Finalize() {
	effects_.clear();
	instance_.reset();
}

void EffectManager::AddEffect(IEffect* effect) {
	effects_.push_back(std::unique_ptr<IEffect>(effect));
}

void EffectManager::EmitHitEffect(const Vector3& position) {
	// 課題要求：回転Plane, Ring, Cylinder, Billboard Planeの共存
	Vector4 color = { 1.0f, 0.8f, 0.2f, 1.0f }; // オレンジ色

	// 1. Ringエフェクト（地面に広がる衝撃波）
	AddEffect(new RingEffect(position, color, 0.5f, 4.0f));

	// 2. Cylinderエフェクト（上に伸びる光の柱）
	AddEffect(new CylinderEffect(position, color, 0.4f, 8.0f, 1.5f));

	// 3. 回転Planeエフェクト（空間を斬るようなエフェクト）
	// 白色でランダムな方向に3つ発生させる
	Vector4 planeColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	for (int i = 0; i < 3; i++) {
		AddEffect(new HitRotPlaneEffect(position, planeColor, 0.3f, 3.0f));
	}

	// 4. Billboard Particle エフェクト（火花）
	AddEffect(new BillboardParticleEffect(position, "HitSpark", 20));
}

void EffectManager::EmitHitPlaneEffect(const Vector3& position) {
	// Plane（板ポリ）のみのヒットエフェクト
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	// ランダムな角度で3つ発生させる
	for (int i = 0; i < 3; i++) {
		AddEffect(new HitRotPlaneEffect(position, color, 0.3f, 3.0f));
	}
}

void EffectManager::EmitDestroyEffect(const Vector3& position) {
	// 敵破壊時：純粋なパーティクルによる大爆発演出
	
	// 幾何学エフェクト（Ring, Cylinder, RotPlane）はチープに見えるため削除し、
	// 代わりにリッチな ExplosionParticleEffect に全振りします。
	
	AddEffect(new ExplosionParticleEffect(position));
}

void EffectManager::EmitMuzzleFlash(const Vector3& position, const Vector3& direction) {
	// パーティクルベースの火花+閃光
	// (板ポリやリングの閃光はStageScene側で機体に追従させて描画する)
	AddEffect(new MuzzleFlashEffect(position, direction));
}

