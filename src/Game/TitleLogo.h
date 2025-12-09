#pragma once
#include "Object3d.h"

class TitleLogo {
public:
	~TitleLogo();
	void Initialize(const MyMath::Vector3& position);
	void Update();
	void Draw();

private:
	Object3d* tutorialObject3d_ = nullptr;
	Object3d* gameObject3d_ = nullptr;
	MyMath::Vector3 basePosition_ = {};
	MyMath::Vector3 gameBasePosition_ = {};
	float amplitude_ = 0.5f;
	float theta_ = 0.0f;
	bool isTutorial_ = true;
};