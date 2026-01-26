#include "Explosion.h"
#include <cassert>

#include "Object3dCommon.h"

void Explosion::Initialize(Model* model, const Vector3& position, Camera* camera) {
	assert(model);
	
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(model);
	object3d_->SetCamera(camera);
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });

	// 爆発の寿命
	timer_ = kLifeTime;
}

void Explosion::Update() {
	timer_--;
	if (timer_ <= 0) {
		isDead_ = true;
	}

	Vector3 scale = object3d_->GetScale();
	float growth = 1.0f;
	scale.x += growth;
	scale.y += growth;
	scale.z += growth;
	object3d_->SetScale(scale);

	// 行列更新
	object3d_->Update();
}

void Explosion::Draw() {
	if (!isDead_ && object3d_) {
		object3d_->Draw();
	}
}