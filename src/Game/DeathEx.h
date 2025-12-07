#pragma once
#include "Object3d.h"
#include "Math/MyMath.h"

class DeathEx {
public:
	~DeathEx();
	void Initialize(const MyMath::Vector3& position, const MyMath::Vector3& rotate);
	void Update();
	void Draw();

private:
	Object3d* object3d_ = nullptr;
	MyMath::Vector4 color_ = { 1,1,1,0 };
	float targetAlpha_ = 1.0f;
	bool isAlphaMax_ = false;
	float t_ = 0.0f;
};