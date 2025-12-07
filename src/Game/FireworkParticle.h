#pragma once
#include "Object3d.h"
#include "Math/MyMath.h"
#include <vector>

class FireworkParticle {
public:
	~FireworkParticle();
	void Initialize(const MyMath::Vector3& position);
	void Update();
	void Draw();
	bool IsFinished() const { return isFinished_; }

private:
	// 1つのクラスで複数の粒(Object3d)を管理
	std::vector<Object3d*> particles_;
	static const int kNumParticles = 16;

	float speed_ = 2.0f;
	float counter_ = 0.0f;
	static inline const float kDuration = 1.0f;
	bool isFinished_ = false;

	MyMath::Vector4 color_ = { 1,1,1,1 };
};