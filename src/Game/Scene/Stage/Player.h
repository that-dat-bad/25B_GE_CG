#pragma once
#include "Object3d.h"
#include "Math/MyMath.h"
#include <memory>
#include <list>
#include <stdint.h>

class PlayerBullet;
class PlayerMissile;
class Enemy;
class Input; 

class Player {
public:
	~Player();
	void Initialize(Model* model, Camera* camera);

	void Update(bool isInputEnable = true);

	void Draw();
	void SetBulletModel(Model* model) { bulletModel_ = model; };
	void SetMissileModel(Model* model) { missileModel_ = model; }

	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }
	const std::list<PlayerMissile*>& GetMissiles() const { return missiles_; }

	Vector3 GetWorldPosition() const;
	Vector3 GetRotation() const;

	int GetHP() const { return hp_; }
	int GetMaxHP() const { return kMaxHP_; }
	bool IsDead() const { return isDead_; }

	int GetLives() const { return lives_; }

	void OnCollision();

	void FireMissile(Enemy* target);

private:
	void Attack();
	std::unique_ptr<Object3d> object3d_ = nullptr;
	Model* bulletModel_ = nullptr;
	Model* missileModel_ = nullptr;

	Camera* camera_ = nullptr;
	Input* input_ = nullptr;
	std::list<PlayerBullet*> bullets_;
	std::list<PlayerMissile*> missiles_;

	static const int kMaxHP_ = 10;
	int hp_ = kMaxHP_;
	bool isDead_ = false;

	static const int kDefaultLives_ = 3;
	int lives_ = kDefaultLives_;

	int invincibleTimer_ = 0;
};

