#include "Bullet.h"

using namespace MyMath;

void Bullet::Spawn(const Vector3& position, const Vector3& direction, float speed, float damage) {
	position_ = position;
	velocity_ = Multiply(speed, direction);
	damage_ = damage;
	lifeTime_ = 5.0f;
	currentTime_ = 0.0f;
	isAlive_ = true;
}

void Bullet::Update(float dt) {
	if (!isAlive_) return;

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
