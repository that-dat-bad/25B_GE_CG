#pragma once
#include "Object3d.h"
#include "Collision.h"
#include "Math/MyMath.h"

class Beam {
public:
	~Beam();
	void Initialize(const MyMath::Vector3& position);
	void Update();
	void Draw();

	MyMath::Vector3 GetWorldPosition() const;
	AABB GetAABB();

private:
	Object3d* object3d_ = nullptr;

	float width_ = 1.6f;
	float height_ = 3.2f;

	MyMath::Vector3 upScale_ = { 1.0f, 0.0f, 0.0f };
	MyMath::Vector3 beamVelocity_ = { -0.5f, 0.0f, 0.0f };
};