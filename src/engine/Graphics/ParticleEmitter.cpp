#include "ParticleEmitter.h"
#include "ParticleManager.h"

// コンストラクタ
ParticleEmitter::ParticleEmitter(const std::string& name, const Transform& transform, uint32_t count, float frequency, const Vector4& color, const Vector3& velocity, float velocityDiff)
	: name_(name), transform_(transform), count_(count), frequency_(frequency), frequencyTime_(0.0f),
	color_(color), velocity_(velocity), velocityDiff_(velocityDiff) { 
}

void ParticleEmitter::Update() {
	frequencyTime_ += 1.0f / 60.0f;

	if (frequency_ <= frequencyTime_) {
		Emit();

		frequencyTime_ -= frequency_;
	}
}

void ParticleEmitter::Emit() {
	// ParticleManagerへパラメータを渡す
	ParticleManager::GetInstance()->Emit(name_, transform_.translate, count_, color_, velocity_, velocityDiff_);
}