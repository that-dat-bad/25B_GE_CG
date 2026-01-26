#pragma once
#include "Object3d.h"
#include "../base/Math/MyMath.h"
#include <memory>

class PlayerBullet {
public:
	// 初期化時、カメラも受け取る必要がある（Object3dにセットするため）
	void Initialize(Model* model, const Vector3& position, const Vector3& velocity, Camera* camera);
	void Update();
	void Draw();

	bool IsDead() const { return isDead_; }
	void OnCollision();

	Vector3 GetWorldPosition() const;

	//攻撃力
	int GetPower() const { return 1; }

private:
	std::unique_ptr<Object3d> object3d_ = nullptr;
	Vector3 velocity_;
	bool isDead_ = false;

	static const int32_t kLifeTime = 60 * 5;
	int32_t deathTimer_ = kLifeTime;
};