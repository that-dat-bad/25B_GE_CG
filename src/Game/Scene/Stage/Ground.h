#pragma once
#include "Object3d.h"
#include "Math/MyMath.h"
#include <memory>
#include <vector>

class Ground {
public:
	void Initialize(Model* model, Camera* camera);
	void Update();
	void Draw();

private:
	Model* model_ = nullptr;

	static const int kGroundCount = 20;

	const float kGroundDepth = 200.0f;
	
	std::vector<std::unique_ptr<Object3d>> object3ds_;
};

