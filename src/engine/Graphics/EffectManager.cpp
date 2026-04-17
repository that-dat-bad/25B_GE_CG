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
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/white1x1.png");

	// ランダムな初期角度と回転速度の決定
	rotationEulers_ = { (float)(rand() % 314) / 100.0f, (float)(rand() % 314) / 100.0f, (float)(rand() % 314) / 100.0f };
	rotationSpeed_ = { 0.1f, 0.05f, 0.2f };
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

	scale_.x = targetSize_ * (1.0f - t);
	scale_.y = targetSize_ * (1.0f - t);
	scale_.z = targetSize_ * (1.0f - t);
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
	AddEffect(new HitRotPlaneEffect(position, color, 0.3f, 3.0f));

	// 4. Billboard Particle エフェクト（火花）
	// (事前に "HitSpark" のようなグループがParticleManagerに作られている前提)
	AddEffect(new BillboardParticleEffect(position, "HitSpark", 20));
}
