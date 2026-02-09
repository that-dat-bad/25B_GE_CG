#include "Enemy.h"
#include "EnemyMissile.h"

#include "Player.h"
#include <cassert>
#include <cmath>
#define _USE_MATH_DEFINES
#include<math.h>
#include "Object3dCommon.h"

void EnemyMissile::Initialize(Model* model, const Vector3& position, Player* target, Camera* camera) {
	assert(model);
	target_ = target;

	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(model);
	object3d_->SetCamera(camera);
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });

	velocity_ = {0.0f, 0.0f, -0.1f};
	deathTimer_ = 60 * 10;
	object3d_->Update();
}

void EnemyMissile::Update() {
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	if (target_ && !target_->IsDead()) {
		Vector3 targetPos = target_->GetWorldPosition();
		Vector3 myPos = object3d_->GetTranslate();
		Vector3 toTarget = {targetPos.x - myPos.x, targetPos.y - myPos.y, targetPos.z - myPos.z};

		float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
		if (dist > 0.0f) {
			toTarget.x /= dist;
			toTarget.y /= dist;
			toTarget.z /= dist;
		}

		Vector3 currentDir = velocity_;
		float speed = std::sqrt(currentDir.x * currentDir.x + currentDir.y * currentDir.y + currentDir.z * currentDir.z);
		if (speed > 0.0f) {
			currentDir.x /= speed;
			currentDir.y /= speed;
			currentDir.z /= speed;
		} else {
			currentDir = {0, 0, 1};
		}

		float dot = currentDir.x * toTarget.x + currentDir.y * toTarget.y + currentDir.z * toTarget.z;
		if (dot < 0.0f) {
			target_ = nullptr;
		} else {
			const float kHomingStrength = 0.03f;
			Vector3 newDir;
			newDir.x = LerpShort(currentDir.x, toTarget.x, kHomingStrength);
			newDir.y = LerpShort(currentDir.y, toTarget.y, kHomingStrength);
			newDir.z = LerpShort(currentDir.z, toTarget.z, kHomingStrength);

			float len = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y + newDir.z * newDir.z);
			if (len > 0.0f) {
				newDir.x /= len;
				newDir.y /= len;
				newDir.z /= len;
			}

			const float kMissileSpeed = 0.5f;
			velocity_ = {newDir.x * kMissileSpeed, newDir.y * kMissileSpeed, newDir.z * kMissileSpeed};

			Vector3 rot = object3d_->GetRotate();
			rot.y = std::atan2(velocity_.x, velocity_.z) + static_cast<float>(M_PI);
			float hLen = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
			rot.x = -std::atan2(velocity_.y, hLen);
			object3d_->SetRotate(rot);
		}
	}

	Vector3 pos = object3d_->GetTranslate();
	pos.x += velocity_.x;
	pos.y += velocity_.y;
	pos.z += velocity_.z;
	object3d_->SetTranslate(pos);

	object3d_->Update();
}

void EnemyMissile::Draw() {
	if(object3d_) object3d_->Draw();
}

void EnemyMissile::OnCollision() { isDead_ = true; }

Vector3 EnemyMissile::GetWorldPosition() const {
	if(object3d_) return object3d_->GetTranslate();
	return {0,0,0};
}
