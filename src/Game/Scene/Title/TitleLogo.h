#pragma once
#include "Object3d.h"
#include <memory>
#include "math/MyMath.h"

class TitleLogo {
private:
	std::unique_ptr<Object3d> object3d_ = nullptr;

public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();
	void SetPosition(const Vector3& position);
};

