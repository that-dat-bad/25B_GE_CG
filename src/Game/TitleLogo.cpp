#include "TitleLogo.h"
#include <cmath>
#include <numbers> // std::numbers::pi_v

TitleLogo::~TitleLogo() {
	if (object3d_) delete object3d_;
}

void TitleLogo::Initialize(const MyMath::Vector3& position) {
	object3d_ = Object3d::Create();
	object3d_->SetModel("titleLogo.obj");
	object3d_->SetTranslate(position);
	basePosition_ = position;
}

void TitleLogo::Update() {
	// ふわふわ動作
	theta_ += 3.14159265f / 60.0f;
	float offsetY = std::sin(theta_) * amplitude_;

	MyMath::Vector3 pos = basePosition_;
	pos.y += offsetY + 2.0f;
	object3d_->SetTranslate(pos);

	object3d_->Update();
}

void TitleLogo::Draw() {
	if (object3d_) object3d_->Draw();
}