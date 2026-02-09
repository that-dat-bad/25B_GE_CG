#pragma once
#include "Sprite.h"
#include "Math/MyMath.h"
#include <memory>
#include "Camera.h"

class Reticle {
public:
	void Initialize();

	void Update(const Vector3& targetWorldPos, const Camera& camera);

	void Draw();

	Vector2 GetPosition() const { return position_; }

private:
	std::unique_ptr<Sprite> sprite_ = nullptr;
	uint32_t textureHandle_ = 0;

	Vector2 position_ = {0, 0};
};

