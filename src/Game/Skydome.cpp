#include "Skydome.h"

Skydome::~Skydome() {
	if (object3d_) delete object3d_;
}

void Skydome::Initialize() {
	object3d_ = Object3d::Create();
	object3d_->SetModel("skydome.obj");
	object3d_->SetScale({ 100.0f, 100.0f, 100.0f }); // 適切なサイズに
}

void Skydome::Update() {
	if (object3d_) object3d_->Update();
}

void Skydome::Draw() {
	if (object3d_) object3d_->Draw();
}