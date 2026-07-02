#pragma once
#include "Collision3DManager.h"
#include "Collision2DManager.h"

/// @brief 衝突判定の統合マネージャ（Facade）
/// 3D・2D の判定マネージャをまとめて管理する
/// Scene 側はこのクラスのみに依存すればよい
class CollisionSystem {
public:
	// ============================================================
	// マネージャへのアクセス
	// ============================================================

	/// @brief 3D判定マネージャを取得
	Collision3DManager& Get3D() { return manager3d_; }
	const Collision3DManager& Get3D() const { return manager3d_; }

	/// @brief 2D判定マネージャを取得
	Collision2DManager& Get2D() { return manager2d_; }
	const Collision2DManager& Get2D() const { return manager2d_; }

	// ============================================================
	// ショートカット（直接登録）
	// ============================================================

	/// @brief 3Dコライダーを登録
	void Register(ICollisionBody3D* body) { manager3d_.Register(body); }

	/// @brief 2Dコライダーを登録
	void Register(ICollisionBody2D* body) { manager2d_.Register(body); }

	// ============================================================
	// フレーム制御
	// ============================================================

	/// @brief 全判定を実行（3D → 2D の順）
	void UpdateAll();

	/// @brief 全登録をクリア（フレーム開始時）
	void ClearAll();

private:
	Collision3DManager manager3d_;
	Collision2DManager manager2d_;
};
