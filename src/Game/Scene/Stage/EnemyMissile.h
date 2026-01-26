#pragma once
#include "Object3d.h"
#include "../base/Math/MyMath.h"
#include <memory>

class Player;

class EnemyMissile {
public:
	void Initialize(Model* model, const Vector3& position, Player* target, Camera* camera);
	void Update();
	void Draw();

	bool IsDead() const { return isDead_; }
	void OnCollision();

	Vector3 GetWorldPosition() const;

	int GetPower() const { return 10; }

private:
	std::unique_ptr<Object3d> object3d_ = nullptr;

	// ★変更: ターゲット
	Player* target_ = nullptr;
	Vector3 velocity_;

	bool isDead_ = false;
	int32_t deathTimer_ = 0;
};