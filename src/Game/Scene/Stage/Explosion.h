#pragma once
#include "Object3d.h"
#include "../base/Math/MyMath.h"
#include <memory>

class Explosion {
public:
	// 初期化（モデルと発生場所を受け取る）
	void Initialize(Model* model, const Vector3& position, Camera* camera);

	// 更新（膨らんでいく）
	void Update();

	// 描画
	void Draw();

	// 死んだかチェック
	bool IsDead() const { return isDead_; }

private:
	std::unique_ptr<Object3d> object3d_ = nullptr;

	bool isDead_ = false;
	int32_t timer_ = 0;

	// 爆発の最大寿命
	static const int32_t kLifeTime = 20;
};