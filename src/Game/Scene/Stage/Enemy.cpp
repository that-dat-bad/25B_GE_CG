#include "Enemy.h"
#include "Player.h"

#include <cassert>
#include <cmath>
#include "Object3dCommon.h"

// 円周率
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


Enemy::~Enemy() {
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

	// 1. タイプ設定 (HPと見た目)
	if (typeStr == "B") {
		type_ = EnemyType::TypeB;
		hp_ = 20;                                    // 硬い
		object3d_->SetScale({ 2.0f, 2.0f, 2.0f });   // デカい
	} else {
		type_ = EnemyType::TypeA;
		hp_ = 3; // 普通
		object3d_->SetScale({ 1.0f, 1.0f, 1.0f });
	}

	// 2. 攻撃パターン設定
	if (patternStr == "Homing") {
		attackPattern_ = AttackPattern::Homing;
	} else if (patternStr == "None") {
		attackPattern_ = AttackPattern::None;
	} else {
		attackPattern_ = AttackPattern::Normal;
	}

	stateFunction_ = &Enemy::UpdateApproach;
	isDead_ = false;

	// 開幕攻撃
	Fire();
}

void Enemy::Update() {
	// 弾更新
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

	// ミサイル更新
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
	// ミサイル描画
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

	// 姿勢制御
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
	Matrix4x4 matRot = MakeAffineMatrix({1, 1, 1}, rot, {0, 0, 0});
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
		// 通常弾
		Vector3 velocity = {0, 0, -0.5f};
		EnemyBullet* newBullet = new EnemyBullet();
		newBullet->Initialize(bulletModel_, position, velocity, camera_, false, nullptr);
		bullets_.push_back(newBullet);
	} else if (attackPattern_ == AttackPattern::Homing) {
		// ミサイル発射
		EnemyMissile* newMissile = new EnemyMissile();
		// プレイヤーをターゲットにする
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