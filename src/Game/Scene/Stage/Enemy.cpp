#include "Enemy.h"
#include "Player.h"
#include "Sprite.h"
#include "SpriteCommon.h"

#include <cassert>
#include <cmath>
#include "Object3dCommon.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


Enemy::~Enemy() {
	delete minimapSprite_; // Delete sprite
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
	for (EnemyMissile* missile : missiles_) {
		delete missile;
	}
}

void Enemy::Initialize(Model* model, const Vector3& position, const Vector3& velocity, const std::string& typeStr, const std::string& patternStr, Camera* camera) {
	assert(model);
	camera_ = camera;
	
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(model);
	object3d_->SetCamera(camera);
	object3d_->SetTranslate(position);

	velocity_ = velocity;

	// Minimap Sprite Init
	minimapSprite_ = new Sprite();
	minimapSprite_->Initialize(SpriteCommon::GetInstance(), "white1x1.png");
	minimapSprite_->SetSize({ 8.0f, 8.0f });
	minimapSprite_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // Red
	minimapSprite_->SetAnchorPoint({ 0.5f, 0.5f });

	if (typeStr == "B") {
		type_ = EnemyType::TypeB;
		hp_ = 20;                                    
		object3d_->SetScale({ 2.0f, 2.0f, 2.0f });   
	} else {
		type_ = EnemyType::TypeA;
		hp_ = 3; 
		object3d_->SetScale({ 1.0f, 1.0f, 1.0f });
	}

	if (patternStr == "Homing") {
		attackPattern_ = AttackPattern::Homing;
	} else if (patternStr == "None") {
		attackPattern_ = AttackPattern::None;
	} else {
		attackPattern_ = AttackPattern::Normal;
	}

	stateFunction_ = &Enemy::UpdateApproach;
	isDead_ = false;

	Fire();
}
//...
void Enemy::DrawMinimap(const Vector2& position) {
	if (minimapSprite_) {
		minimapSprite_->SetPosition(position);
		minimapSprite_->Update();
		minimapSprite_->Draw();
	}
}

void Enemy::Update() {
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});
	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}

	missiles_.remove_if([](EnemyMissile* missile) {
		if (missile->IsDead()) {
			delete missile;
			return true;
		}
		return false;
	});
	for (EnemyMissile* missile : missiles_) {
		missile->Update();
	}

	(this->*stateFunction_)();
	
	Vector3 pos = object3d_->GetTranslate();

	if (pos.z < -10.0f || pos.z >1000.0f) {
		isDead_ = true;
	}
	object3d_->Update();
}

void Enemy::Draw() {
	if(object3d_) object3d_->Draw();
	
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw();
	}
	for (EnemyMissile* missile : missiles_) {
		missile->Draw();
	}
}

void Enemy::UpdateApproach() {
	Vector3 pos = object3d_->GetTranslate();
	pos.x += velocity_.x;
	pos.y += velocity_.y;
	pos.z += velocity_.z;
	object3d_->SetTranslate(pos);

	Vector3 rot = object3d_->GetRotate();
	float targetRotY = std::atan2(velocity_.x, velocity_.z) + static_cast<float>(M_PI);
	float targetRotZ = -velocity_.x * 2.0f;
	float targetRotX = -velocity_.y * 2.0f;

	const float kTiltSpeed = 0.1f;
	rot.y = LerpShort(rot.y, targetRotY, kTiltSpeed);
	rot.z = LerpShort(rot.z, targetRotZ, kTiltSpeed);
	rot.x = LerpShort(rot.x, targetRotX, kTiltSpeed);
	object3d_->SetRotate(rot);

	if (pos.z < 0.0f) {
		stateFunction_ = &Enemy::UpdateLeave;
	}
}

void Enemy::UpdateLeave() {
	Vector3 rot = object3d_->GetRotate();
	rot.y += 0.02f;
	rot.x += 0.01f;
	rot.z += 0.02f;
	object3d_->SetRotate(rot);
	
	Vector3 move = {0, 0, -0.4f};
	Vector3 scale = { 1.0f, 1.0f, 1.0f };
	Vector3 translate = { 0.0f, 0.0f, 0.0f };
	Matrix4x4 matRot = MakeAffineMatrix(scale, rot, translate);
	move = TransformNormal(move, matRot);
	
	Vector3 pos = object3d_->GetTranslate();
	pos.x += move.x;
	pos.y += move.y;
	pos.z += move.z;
	object3d_->SetTranslate(pos);
}

void Enemy::Fire() {
	if (attackPattern_ == AttackPattern::None)
		return;

	Vector3 position = object3d_->GetTranslate();

	if (attackPattern_ == AttackPattern::Normal) {
		Vector3 velocity = {0, 0, -0.5f};
		EnemyBullet* newBullet = new EnemyBullet();
		newBullet->Initialize(bulletModel_, position, velocity, camera_, false, nullptr);
		bullets_.push_back(newBullet);
	} else if (attackPattern_ == AttackPattern::Homing) {
		EnemyMissile* newMissile = new EnemyMissile();
		newMissile->Initialize(missileModel_, position, player_, camera_);
		missiles_.push_back(newMissile);
	}
}

void Enemy::OnCollision(int damage) {
	hp_ -= damage;
	if (hp_ <= 0) {
		isDead_ = true;
	}
}

Vector3 Enemy::GetWorldPosition() const {
	if (object3d_) return object3d_->GetTranslate();
	return {0,0,0};
}
