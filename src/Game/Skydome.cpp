#include "Skydome.h"
#include"Model.h"

Skydome::~Skydome() {
	// unique_ptr により自動解放
}

void Skydome::Initialize() {
	std::string path = "./Resources/Skydome/Skydome.obj";
	Model::LoadFromOBJ(path);
	object3d_.reset(Object3d::Create());
	object3d_->SetModel(path);
	object3d_->SetScale({ 10.0f, 10.0f, 10.0f });
}

void Skydome::Update() {
	if (object3d_) object3d_->Update();
}

void Skydome::Draw() {
	if (object3d_) object3d_->Draw();
}