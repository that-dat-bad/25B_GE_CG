#include "PlayerBullet.h"
#include <cassert>
#include "Object3dCommon.h"

void PlayerBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity, Camera* camera) {
	assert(model);
	
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(model);
	object3d_->SetCamera(camera);
	object3d_->SetTranslate(position);

	velocity_ = velocity;
	deathTimer_ = kLifeTime;
}

void PlayerBullet::Update() {
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
	
	Vector3 pos = object3d_->GetTranslate();
	pos.x += velocity_.x;
	pos.y += velocity_.y;
	pos.z += velocity_.z;
	object3d_->SetTranslate(pos);

	object3d_->Update();
}

void PlayerBullet::Draw() {
	if (object3d_) {
		object3d_->Draw();
	}
}

void PlayerBullet::OnCollision() { isDead_ = true; }

Vector3 PlayerBullet::GetWorldPosition() const {
	if (object3d_) {
		return object3d_->GetTranslate();
	}
	return { 0,0,0 };
}