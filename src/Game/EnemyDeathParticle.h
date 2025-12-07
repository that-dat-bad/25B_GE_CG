#pragma once
#include "Object3d.h"
#include "Math/MyMath.h"

class EnemyDeathParticle {
public:
	~EnemyDeathParticle();
	void Initialize(const MyMath::Vector3& position);
	void Update();
	void Draw();

private:
	Object3d* object3d_ = nullptr;
	MyMath::Vector4 color_ = { 1,1,1,1 };
	float targetAlpha_ = 1.0f;
	bool isAlphaMax_ = false;
	float t_ = 0.0f;
};