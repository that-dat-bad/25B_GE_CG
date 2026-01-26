#pragma once
#include "Sprite.h"
#include "../base/Math/MyMath.h"
#include <memory>
#include "Camera.h"

class Reticle {
public:
	// 初期化
	void Initialize();

	// 更新
	void Update(const Vector3& targetWorldPos, const Camera& camera);

	// 描画
	void Draw();

	Vector2 GetPosition() const { return position_; }

private:
	// スプライト
	std::unique_ptr<Sprite> sprite_ = nullptr;
	uint32_t textureHandle_ = 0;

	// 画面上の位置
	Vector2 position_ = {0, 0};
};