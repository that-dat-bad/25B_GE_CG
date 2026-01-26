#pragma once
#include "Object3d.h"
#include "../base/Math/MyMath.h"
#include <memory>
#include <list>
#include <stdint.h>

// 前方宣言
class PlayerBullet;
class PlayerMissile;
class Enemy;
class Input; // Inputを使うなら前方宣言、ポインタで保持

class Player {
public:
	~Player();
	// CameraはObject3dにセットするためにポインタで受け取る
	void Initialize(Model* model, Camera* camera);

	void Update(bool isInputEnable = true);

	void Draw();
	void SetBulletModel(Model* model) { bulletModel_ = model; };
	void SetMissileModel(Model* model) { missileModel_ = model; }

	// リスト取得
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }
	const std::list<PlayerMissile*>& GetMissiles() const { return missiles_; }

	// 座標・回転取得
	Vector3 GetWorldPosition() const;
	Vector3 GetRotation() const;

	// HP関連のゲッター
	int GetHP() const { return hp_; }
	int GetMaxHP() const { return kMaxHP_; }
	bool IsDead() const { return isDead_; }

	// 残機数の取得
	int GetLives() const { return lives_; }

	// 衝突処理
	void OnCollision();

	// ミサイル発射
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

	// HPパラメータ
	static const int kMaxHP_ = 10;
	int hp_ = kMaxHP_;
	bool isDead_ = false;

	// 残機
	static const int kDefaultLives_ = 3;
	int lives_ = kDefaultLives_;

	// 無敵時間タイマー
	int invincibleTimer_ = 0;
};