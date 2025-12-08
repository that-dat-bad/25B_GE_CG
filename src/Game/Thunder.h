#pragma once
#include "Object3d.h"
#include "Collision.h"

class Thunder {
public:
	~Thunder();
	void Initialize(const MyMath::Vector3& position);
	void Update();
	void Draw();

	MyMath::Vector3 GetWorldPosition() const;
	AABB GetAABB();
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

private:
	Object3d* object3d_ = nullptr;
	MyMath::Vector4 color_ = { 1,1,1,1 };
	float targetAlpha_ = 1.0f;
	bool isAlphaMax_ = false;
	bool isCollisionDisabled_ = false;

	float width_ = 12.8f;
	float height_ = 80.0f;
	float t_ = 0.0f;
};