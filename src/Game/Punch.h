#pragma once
#include "Object3d.h"
#include "Collision.h"
#include "Math/MyMath.h"

class Punch {
public:
	~Punch();

	// 初期化
	// punched: 最初から突き出している状態かどうかのフラグ
	void Initialize(const MyMath::Vector3& position, int punched);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 位置を設定
	void SetPosition(const MyMath::Vector3& position);

	// ゲッター
	MyMath::Vector3 GetWorldPosition() const;
	AABB GetAABB();

private:
	// TDEngine用 Object3d
	Object3d* object3d_ = nullptr;

	// キャラクターの当たり判定サイズ
	float width_ = 1.6f;
	float height_ = 1.6f;

	// イージング用変数
	float t_ = 0.0f;

	// パンチ済みフラグ
	bool isPunched_ = false;
	// パンチした後の拳の位置（目標地点）
	MyMath::Vector3 punchedPosition_ = {};
};