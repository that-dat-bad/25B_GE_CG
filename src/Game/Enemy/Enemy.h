#pragma once
#include <memory>
#include <string>
#include "../../engine/base/Math/MyMath.h"
#include "../../engine/Graphics/Model/Object3d.h"
#include "../../engine/Physics/ICollisionBody.h"
#include "../../engine/Physics/CollisionConfig.h"
#include "../FlightModel/FlightModel.h"
#include "../FlightModel/Payload/Gunpod.h"
#include "../Bullet/BulletManager.h"
#include "../FlightModel/MouseAimController.h"

class Object3dCommon;
class Camera;

/// @brief 空中ターゲット（敵AI）
class Enemy : public ICollisionBody3D {
public:
	Enemy() = default;
	~Enemy() = default;

	/// @brief 初期化
	/// @param position 配置座標
	/// @param modelPath モデルファイルパス
	/// @param maxHealth 最大HP
	/// @param airframeData 機体パラメータ
	/// @param engineData エンジンパラメータ
	/// @param gunpodData ガンポッドパラメータ
	/// @param playerFlightModel 自機のフライトモデルへのポインタ
	/// @param bulletManager 弾丸マネージャーへのポインタ
	void Initialize(
		const MyMath::Vector3& position, 
		const std::string& modelPath, 
		float maxHealth,
		const AirframeData& airframeData,
		const EngineData& engineData,
		const GunPodData& gunpodData,
		FlightModel* playerFlightModel,
		BulletManager* bulletManager
	);

	/// @brief 更新処理
	void Update(float deltaTime);

	/// @brief 描画処理
	void Draw();

	/// @brief ダメージを受ける
	/// @param damage ダメージ量
	void TakeDamage(float damage);

	// === アクセッサ ===
	bool IsAlive() const { return isAlive_; }
	MyMath::Vector3 GetPosition() const { return flightModel_.GetPosition(); }
	float GetCollisionRadius() const { return collisionRadius_; }
	float GetHealth() const { return health_; }
	float GetMaxHealth() const { return maxHealth_; }

	// === ICollisionBody3D 実装 ===
	SphereCollider GetSphereCollider() const override {
		return { flightModel_.GetPosition(), collisionRadius_ };
	}
	uint32_t GetCollisionAttribute() const override { return CollisionAttribute::kEnemy; }
	uint32_t GetCollisionMask() const override { return CollisionMask::kEnemy; }
	void OnCollision(ICollisionBody3D* other) override;
	bool IsCollisionActive() const override { return isAlive_; }

private:
	/// @brief AI思考・操縦入力の更新
	void UpdateAI(float deltaTime);

	// 物理モデルとAIコントローラー
	FlightModel flightModel_;
	MouseAimController aimController_;
	GunPod gunpod_;

	// 参照ポインタ
	FlightModel* playerFlightModel_ = nullptr;
	BulletManager* bulletManager_ = nullptr;

	float health_ = 0.0f;
	float maxHealth_ = 0.0f;
	bool isAlive_ = true;

	// 衝突判定用の半径
	float collisionRadius_ = 5.0f;

	// エフェクト関連
	float muzzleFlashTimer_ = 0.0f;
	int muzzleFlashCount_ = 0;

	// 描画用3Dオブジェクト
	std::unique_ptr<Object3d> object3d_ = nullptr;
};
