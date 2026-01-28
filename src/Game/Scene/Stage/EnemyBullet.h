#pragma once
#include "Object3d.h"
#include "Math/MyMath.h"
#include <memory>

class Player; 

class EnemyBullet {
public:
	void Initialize(Model* model, const Vector3& position, const Vector3& velocity, Camera* camera, bool isHoming = false, Player* target = nullptr);

	void Update();
	void Draw();

	bool IsDead() const { return isDead_; }
	void OnCollision() { isDead_ = true; }
	Vector3 GetWorldPosition() const;

private:
	std::unique_ptr<Object3d> object3d_ = nullptr;
	Vector3 velocity_;

	bool isDead_ = false;
	int32_t deathTimer_ = 0;
	static const int32_t kLifeTime = 60 * 5;

	bool isHoming_ = false;
	Player* target_ = nullptr;
};

