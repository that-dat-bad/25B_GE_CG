#pragma once
#include "Object3d.h"

class TitleLogo {
public:
	~TitleLogo();
	void Initialize(const MyMath::Vector3& position);
	void Update();
	void Draw();

private:
	Object3d* object3d_ = nullptr;
	MyMath::Vector3 basePosition_ = {};
	float amplitude_ = 0.5f;
	float theta_ = 0.0f;
};