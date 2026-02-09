#pragma once
#include "EnemyBullet.h"
#include "EnemyMissile.h"
#include "Object3d.h"
#include "Math/MyMath.h"
#include <memory>
#include <list>
#include <string>

class Player;
class Sprite;

enum class EnemyType { TypeA, TypeB };
enum class AttackPattern { None, Normal, Homing };

class Enemy {
public:
	~Enemy();

	void Initialize(Model* model, const Vector3& position, const Vector3& velocity, const std::string& typeStr, const std::string& patternStr, Camera* camera);

	void Update();
	void Draw();

	void SetPlayer(Player* player) { player_ = player; }
	void SetBulletModel(Model* model) { bulletModel_ = model; }
	void SetMissileModel(Model* model) { missileModel_ = model; }

	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }
	const std::list<EnemyMissile*>& GetMissiles() const { return missiles_; }

	Vector3 GetWorldPosition() const;

	void OnCollision(int damage);
	bool IsDead() const { return isDead_; }

	void DrawMinimap(const Vector2& position);

private:
	void UpdateApproach();
	void UpdateLeave();
	void Fire();

	std::unique_ptr<Object3d> object3d_ = nullptr;
	Model* bulletModel_ = nullptr;
	Model* missileModel_ = nullptr;
	Sprite* minimapSprite_ = nullptr;

	Player* player_ = nullptr;
	Camera* camera_ = nullptr; 
	Vector3 velocity_;

	std::list<EnemyBullet*> bullets_;
	std::list<EnemyMissile*> missiles_;

	void (Enemy::*stateFunction_)() = nullptr;

	int hp_ = 0;
	bool isDead_ = false;
	EnemyType type_ = EnemyType::TypeA;
	AttackPattern attackPattern_ = AttackPattern::None;
};

