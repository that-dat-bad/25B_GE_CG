#pragma once
#include "Object3d.h"
#include "Collision.h"

class Needle {
public:
	~Needle();
	void Initialize(const MyMath::Vector3& position, const MyMath::Vector3& rotate);
	void Update();
	void Draw();

	MyMath::Vector3 GetWorldPosition() const;
	OBB GetOBB();

private:
	Object3d* object3d_ = nullptr;
	MyMath::Vector3 upScale_ = { 60.0f, 1.0f, 1.0f };
	float width_ = 1.6f;
	float height_ = 1.6f;
	float t_ = 0.0f;
};