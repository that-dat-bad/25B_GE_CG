#pragma once
#include "Object3d.h"
#include "Collision.h"

class Wind {
public:
	~Wind();
	void Initialize(const MyMath::Vector3& position);
	void Update();
	void Draw();

	MyMath::Vector3 GetWorldPosition() const;
	AABB GetAABB();
	MyMath::Vector3 GetVelocity() const;

private:
	Object3d* object3d_ = nullptr;
	MyMath::Vector3 direction_ = { -1.0f, 0.0f, 0.0f };
	float strength_ = 1.0f;
	MyMath::Vector3 range_ = { 48.0f, 6.0f, 2.0f };
};