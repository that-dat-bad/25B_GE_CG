#include "BackGround.h"
#include"ModelManager.h"
BackGround::~BackGround() {
	if (object3d_) delete object3d_;
}

void BackGround::Initialize(const std::string& modelName, const MyMath::Vector3& position) {
	std::string path = "Resources/" + modelName + "/" + modelName + ".obj";
	ModelManager::GetInstance()->LoadModel(path);
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });
}

void BackGround::Update() {
	if (object3d_) object3d_->Update();
}

void BackGround::Draw() {
	if (object3d_) object3d_->Draw();
}