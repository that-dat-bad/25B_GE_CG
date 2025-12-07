#include "BackGround.h"

BackGround::~BackGround() {
	if (object3d_) delete object3d_;
}

void BackGround::Initialize(const std::string& modelName, const MyMath::Vector3& position) {
	object3d_ = Object3d::Create();
	// 拡張子 .obj を付与してセット (TDEngineの仕様に合わせて調整)
	object3d_->SetModel(modelName + ".obj");
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });
}

void BackGround::Update() {
	if (object3d_) object3d_->Update();
}

void BackGround::Draw() {
	if (object3d_) object3d_->Draw();
}