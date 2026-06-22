#include "Bullet.h"

using namespace MyMath;

void Bullet::Spawn(const Vector3& position, const Vector3& direction, float speed, float damage) {
	position_ = position;
	velocity_ = Multiply(speed, direction);
	damage_ = damage;
	lifeTime_ = 5.0f;
	currentTime_ = 0.0f;
	isAlive_ = true;

	// 履歴の初期化
	for (int i = 0; i < kMaxHistory; ++i) {
		history_[i] = position;
	}
	historyCount_ = 1;
}

void Bullet::Update(float dt) {
	if (!isAlive_) return;

	// 履歴の更新
	for (int i = kMaxHistory - 1; i > 0; --i) {
		history_[i] = history_[i - 1];
	}
	history_[0] = position_;
	if (historyCount_ < kMaxHistory) {
		historyCount_++;
	}

	// 位置を更新
	position_ = Add(position_, Multiply(dt, velocity_));

	// 寿命チェック
	currentTime_ += dt;
	if (currentTime_ >= lifeTime_) {
		isAlive_ = false;
	}
}

SphereCollider Bullet::GetCollider() const {
	SphereCollider collider;
	collider.center = position_;
	collider.radius = kCollisionRadius;
	return collider;
}
