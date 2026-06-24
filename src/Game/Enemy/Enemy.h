#pragma once
#include <memory>
#include <string>
#include "../../engine/base/Math/MyMath.h"
#include "../../engine/Graphics/Object3d.h"

class Object3dCommon;
class Camera;

/// @brief 静止地上ターゲット（敵）
class Enemy {
public:
	Enemy() = default;
	~Enemy() = default;

	/// @brief 初期化
	/// @param position 配置座標
	/// @param modelPath モデルファイルパス
	/// @param maxHealth 最大HP
	void Initialize(const MyMath::Vector3& position, const std::string& modelPath, float maxHealth);

	/// @brief 更新処理
	void Update();

	/// @brief 描画処理
	void Draw();

	/// @brief ダメージを受ける
	/// @param damage ダメージ量
	void TakeDamage(float damage);

	// === アクセッサ ===
	bool IsAlive() const { return isAlive_; }
	MyMath::Vector3 GetPosition() const { return position_; }
	float GetCollisionRadius() const { return collisionRadius_; }
	float GetHealth() const { return health_; }
	float GetMaxHealth() const { return maxHealth_; }

private:
	// 座標
	MyMath::Vector3 position_{};

	float health_ = 0.0f;
	float maxHealth_ = 0.0f;
	bool isAlive_ = true;

	// 衝突判定用の半径
	float collisionRadius_ = 5.0f;

	// 描画用3Dオブジェクト
	std::unique_ptr<Object3d> object3d_ = nullptr;
};
