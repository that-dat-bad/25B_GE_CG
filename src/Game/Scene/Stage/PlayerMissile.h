#pragma once
#include "Object3d.h"
#include "Math/MyMath.h"
#include <memory>

class Enemy;
class Camera;

class PlayerMissile {
public:
	void Initialize(Model* model, const Vector3& position, Enemy* target, Camera* camera);
	void Update();
	void Draw();
	
	static void EditOffset();
	static Vector3 GetOffset();

	bool IsDead() const { return isDead_; }
	void OnCollision();

	Vector3 GetWorldPosition() const;

	int GetPower() const { return 10; }

	// ターゲットが指定した敵と一致する場合、ターゲットをクリアする
	void ClearTargetIfMatches(Enemy* enemy) {
		if (target_ == enemy) {
			target_ = nullptr;
		}
	}

private:
	std::unique_ptr<Object3d> object3d_ = nullptr;
	Enemy* target_ = nullptr; 
	Vector3 velocity_;

	bool isDead_ = false;
	int32_t deathTimer_ = 0;
};

