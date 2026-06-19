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
	flashScale_ = 50.0f; // 最初からデカい
	flashAlpha_ = 1.0f;
	
	// --- 2. Shockwave (衝撃波): 超高速で広がる ---
	shockwaveScale_ = 5.0f;
	shockwaveAlpha_ = 0.8f;
	
	lightIntensity_ = 200.0f; // 強烈な光

	// --- 3. Core (炎): 爆発的な初速を持つ火柱（短寿命） ---
	// 中心から放射状に飛び散る、濃いオレンジ〜赤の火球
	for (int i = 0; i < 20; i++) {
		Vector3 offset = { ((rand() % 100) / 50.0f - 1.0f) * 1.5f,
		                   ((rand() % 100) / 50.0f - 1.0f) * 1.5f,
		                   ((rand() % 100) / 50.0f - 1.0f) * 1.5f };

		ParticleParameters fireParams;
		// 非常に速い初速で外側に弾け飛ぶ（マイナス値でのmin/max逆転を防ぐためSpread方式に変更）
		Vector3 baseVel = { offset.x * 6.5f, offset.y * 6.5f + 7.5f, offset.z * 6.5f };
		Vector3 spread = { std::abs(offset.x * 1.5f) + 0.1f, std::abs(offset.y * 1.5f) + 2.5f, std::abs(offset.z * 1.5f) + 0.1f };
		fireParams.minVelocity = { baseVel.x - spread.x, baseVel.y - spread.y, baseVel.z - spread.z };
		fireParams.maxVelocity = { baseVel.x + spread.x, baseVel.y + spread.y, baseVel.z + spread.z };

		fireParams.minColor = { 1.0f, 0.3f, 0.0f, 1.0f }; // 最小値
		fireParams.maxColor = { 1.0f, 0.9f, 0.2f, 1.0f }; // 最大値
		fireParams.minLifeTime = 0.2f;
		fireParams.maxLifeTime = 0.4f; // 一瞬で消える
		fireParams.acceleration = { 0.0f, 0.0f, 0.0f };   
		fireParams.minScale = 5.0f;
		fireParams.maxScale = 8.0f;
		fireParams.endScale = 0.0f; // 急速に縮んで消える
		fireParams.fadeOut = true;
		fireParams.scaleEasing = 0.2f;
		pm->Emit("ExplosionFire", position_, fireParams, 1);
	}

	// --- 4. Core (煙): 厚みのある不透明な雲（もくもく感） ---
	// 輪郭が見えないよう、透明度をやや落として枚数を増やし、柔らかく重ねる
	for (int i = 0; i < 30; i++) {
		// 爆発の勢いで少し上に偏って発生
		Vector3 offset = { ((rand() % 100) / 50.0f - 1.0f) * 4.0f,
		                   ((rand() % 100) / 50.0f) * 6.0f,
		                   ((rand() % 100) / 50.0f - 1.0f) * 4.0f };

		ParticleParameters smokeParams;
		// 初速は遅め（爆発地点に留まってモクモク膨張する）
		Vector3 baseVel = { offset.x * 0.5f, offset.y * 0.5f + 1.0f, offset.z * 0.5f };
		Vector3 sSpread = { std::abs(offset.x * 0.2f) + 0.1f, std::abs(offset.y * 0.2f) + 1.0f, std::abs(offset.z * 0.2f) + 0.1f };
		smokeParams.minVelocity = { baseVel.x - sSpread.x, baseVel.y - sSpread.y, baseVel.z - sSpread.z };
		smokeParams.maxVelocity = { baseVel.x + sSpread.x, baseVel.y + sSpread.y, baseVel.z + sSpread.z };
		
		// 輪郭を目立たなくするため、Alphaを0.2〜0.4程度に落とし、重ねて濃さを出す
		float shade = 0.1f + ((rand() % 100) / 100.0f) * 0.2f; // ランダムな濃淡
		smokeParams.minColor = { shade, shade, shade, 0.2f };  
		smokeParams.maxColor = { shade + 0.1f, shade + 0.1f, shade + 0.1f, 0.4f };     
		smokeParams.minLifeTime = 1.0f;
		smokeParams.maxLifeTime = 1.5f;
		smokeParams.acceleration = { 0.0f, 2.0f, 0.0f };  // ゆっくり上昇
		
		smokeParams.minScale = 8.0f;
		smokeParams.maxScale = 14.0f;
		smokeParams.endScale = 25.0f; // 大きく膨張させることでエッジをさらにボカす
		smokeParams.fadeOut = true;
		smokeParams.scaleEasing = 0.8f;  
		pm->Emit("ExplosionSmoke", Add(position_, offset), smokeParams, 2); 
	}

	// --- 5. Debris (破片): 重く速い破片 ---
	int debrisCount = 6 + (rand() % 4); 
	for (int i = 0; i < debrisCount; i++) {
		Debris d;
		d.pos = position_;
		// 斜め上方向に極めて速い初速
		d.vel.x = ((rand() % 100) / 50.0f - 1.0f) * 35.0f;
		d.vel.y = ((rand() % 100) / 100.0f) * 30.0f + 15.0f; 
		d.vel.z = ((rand() % 100) / 50.0f - 1.0f) * 35.0f;
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
		flashScale_ += 100.0f * dt;
		flashAlpha_ -= 10.0f * dt; // 0.1秒で消滅
		if (flashAlpha_ < 0.0f) flashAlpha_ = 0.0f;
	}

	// --- Shockwave (衝撃波): 超高速で一瞬で消える ---
	if (shockwaveAlpha_ > 0.0f) {
		shockwaveScale_ += 800.0f * dt; // 爆発的な速度で広がる
		shockwaveAlpha_ -= 6.0f * dt;   // 0.15秒で消滅
		if (shockwaveAlpha_ < 0.0f) shockwaveAlpha_ = 0.0f;
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
		trailParams.minColor = { 0.1f, 0.1f, 0.1f, 0.4f }; // 濃い黒煙
		trailParams.maxColor = { 0.2f, 0.2f, 0.2f, 0.6f };
		trailParams.minLifeTime = 0.4f;
		trailParams.maxLifeTime = 0.8f;
		trailParams.minScale = 2.0f;
		trailParams.maxScale = 3.0f;
		trailParams.endScale = 6.0f;
		trailParams.fadeOut = true;
		trailParams.scaleEasing = 0.5f;
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

	// --- Shockwave (衝撃波) ---
	// 水平方向に広がるシャープなリング
	if (shockwaveAlpha_ > 0.0f) {
		uint32_t ringTex = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/circle.png");
		Vector4 shockColor = { 1.0f, 0.9f, 0.6f, shockwaveAlpha_ * 0.8f }; 
		Vector3 scale = { shockwaveScale_, shockwaveScale_, shockwaveScale_ };
		
		Vector3 rot = { 0.0f, 0.0f, 0.0f }; 
		prim->DrawRing(scale, rot, position_, shockColor, ringTex, camera, BlendMode::kAdd);
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
	// 敵破壊時：爆発パーティクル + 幾何学エフェクトの複合演出
	Vector4 fireColor = { 1.0f, 0.5f, 0.1f, 1.0f };  // 炎のオレンジ

	// 1. 大きなRingエフェクト（衝撃波）
	AddEffect(new RingEffect(position, fireColor, 0.8f, 8.0f));

	// 2. Cylinderエフェクト（爆発柱）
	AddEffect(new CylinderEffect(position, fireColor, 0.6f, 12.0f, 3.0f));

	// 3. 回転Planeエフェクト（破片散乱風）
	Vector4 whiteColor = { 1.0f, 1.0f, 0.8f, 1.0f };
	for (int i = 0; i < 5; i++) {
		AddEffect(new HitRotPlaneEffect(position, whiteColor, 0.5f, 5.0f));
	}

	// 4. 爆発パーティクルエフェクト（火花+炎+煙の複合演出）
	AddEffect(new ExplosionParticleEffect(position));
}

void EffectManager::EmitMuzzleFlash(const Vector3& position, const Vector3& direction) {
	// パーティクルベースの火花+閃光
	// (板ポリやリングの閃光はStageScene側で機体に追従させて描画する)
	AddEffect(new MuzzleFlashEffect(position, direction));
}

