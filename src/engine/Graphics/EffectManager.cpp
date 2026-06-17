#include "EffectManager.h"
#include "TextureManager.h"

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
// ExplosionParticleEffect
// ============================================
ExplosionParticleEffect::ExplosionParticleEffect(const Vector3& pos) {
	position_ = pos;
	lifeTime_ = 0.1f; // 発行するだけなので短く

	ParticleManager* pm = ParticleManager::GetInstance();

	// === 1. 火花 (Sparks): 高速で放射状に飛散する小さな黄色パーティクル ===
	{
		ParticleParameters sparkParams;
		sparkParams.minVelocity = { -0.8f, -0.3f, -0.8f };
		sparkParams.maxVelocity = {  0.8f,  1.2f,  0.8f };
		sparkParams.minColor = { 1.0f, 0.8f, 0.2f, 1.0f };  // 黄色
		sparkParams.maxColor = { 1.0f, 1.0f, 0.6f, 1.0f };  // 白黄色
		sparkParams.minLifeTime = 0.3f;
		sparkParams.maxLifeTime = 0.8f;
		sparkParams.acceleration = { 0.0f, -0.02f, 0.0f };  // 微量の重力
		sparkParams.minScale = 0.3f;
		sparkParams.maxScale = 0.6f;
		sparkParams.endScale = 0.0f;
		sparkParams.fadeOut = true;
		sparkParams.scaleEasing = 0.5f;  // 急速に縮小
		pm->Emit("ExplosionSpark", position_, sparkParams, 50);
	}

	// === 2. 炎コア (FireCore): 中心付近でゆっくり膨張するオレンジ～赤 ===
	{
		ParticleParameters fireParams;
		fireParams.minVelocity = { -0.15f, 0.05f, -0.15f };
		fireParams.maxVelocity = {  0.15f, 0.3f,   0.15f };
		fireParams.minColor = { 1.0f, 0.3f, 0.0f, 1.0f };  // 深いオレンジ
		fireParams.maxColor = { 1.0f, 0.6f, 0.1f, 1.0f };  // 明るいオレンジ
		fireParams.minLifeTime = 0.4f;
		fireParams.maxLifeTime = 1.0f;
		fireParams.acceleration = { 0.0f, 0.01f, 0.0f };   // ゆっくり上昇
		fireParams.minScale = 1.5f;
		fireParams.maxScale = 3.0f;
		fireParams.endScale = 0.2f;
		fireParams.fadeOut = true;
		fireParams.scaleEasing = 2.0f;  // 後半で急速縮小
		pm->Emit("ExplosionFire", position_, fireParams, 20);
	}

	// === 3. 煙 (Smoke): ゆっくり上昇する灰色パーティクル ===
	{
		ParticleParameters smokeParams;
		smokeParams.minVelocity = { -0.08f, 0.1f, -0.08f };
		smokeParams.maxVelocity = {  0.08f, 0.3f,  0.08f };
		smokeParams.minColor = { 0.3f, 0.3f, 0.3f, 0.6f };  // 暗い灰色
		smokeParams.maxColor = { 0.5f, 0.5f, 0.5f, 0.8f };  // 明るい灰色
		smokeParams.minLifeTime = 1.0f;
		smokeParams.maxLifeTime = 2.0f;
		smokeParams.acceleration = { 0.0f, 0.005f, 0.0f };  // 微量上昇
		smokeParams.minScale = 2.0f;
		smokeParams.maxScale = 4.0f;
		smokeParams.endScale = 5.0f;   // 拡散して消える
		smokeParams.fadeOut = true;
		smokeParams.scaleEasing = 0.5f;  // 最初に急拡大
		pm->Emit("ExplosionSmoke", position_, smokeParams, 15);
	}
}

void ExplosionParticleEffect::Update() {
	currentTime_ += 1.0f / 60.0f;
	if (currentTime_ >= lifeTime_) isDead_ = true;
}

void ExplosionParticleEffect::Draw(Camera* camera) {
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

