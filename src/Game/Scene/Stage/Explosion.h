#pragma once
#include "Object3d.h"
#include "Math/MyMath.h"
#include <memory>

class Explosion {
public:
	void Initialize(Model* model, const Vector3& position, Camera* camera);

	void Update();

	void Draw();

	bool IsDead() const { return isDead_; }

private:
	std::unique_ptr<Object3d> object3d_ = nullptr;

	bool isDead_ = false;
	int32_t timer_ = 0;

	static const int32_t kLifeTime = 20;
};

