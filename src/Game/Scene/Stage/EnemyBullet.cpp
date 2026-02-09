#include "EnemyBullet.h"
#include "Player.h"
#include <cassert>
#include <cmath>
#include "Object3dCommon.h"

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity, Camera* camera, bool isHoming, Player* target) {
	assert(model);
	
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(model);
	object3d_->SetCamera(camera);
	object3d_->SetTranslate(position);

	velocity_ = velocity;

	isHoming_ = isHoming;
	target_ = target;

	deathTimer_ = kLifeTime;
	object3d_->Update();
}

void EnemyBullet::Update() {
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	if (isHoming_ && target_) {
		Vector3 targetPos = target_->GetWorldPosition();
		Vector3 myPos = object3d_->GetTranslate();
		Vector3 toTarget = {targetPos.x - myPos.x, targetPos.y - myPos.y, targetPos.z - myPos.z};

		float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
		if (dist > 0) {
			toTarget.x /= dist;
			toTarget.y /= dist;
			toTarget.z /= dist;
		}

		const float kHomingStrength = 0.05f;
		velocity_.x = LerpShort(velocity_.x, toTarget.x * 0.5f, kHomingStrength);
		velocity_.y = LerpShort(velocity_.y, toTarget.y * 0.5f, kHomingStrength);
		velocity_.z = LerpShort(velocity_.z, toTarget.z * 0.5f, kHomingStrength);
	}

	Vector3 pos = object3d_->GetTranslate();
	pos.x += velocity_.x;
	pos.y += velocity_.y;
	pos.z += velocity_.z;
	object3d_->SetTranslate(pos);

	object3d_->Update();
}

void EnemyBullet::Draw() { 
	if(object3d_) object3d_->Draw();
}

Vector3 EnemyBullet::GetWorldPosition() const {
	if (object3d_) return object3d_->GetTranslate();
	return { 0,0,0 };
}
