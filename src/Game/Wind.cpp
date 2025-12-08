#include "Wind.h"
#include "Model.h"
using namespace MyMath;

Wind::~Wind() {
	if (object3d_) delete object3d_;
}

void Wind::Initialize(const Vector3& position) {
	std::string path = "./Resources/wind/wind.obj";
	Model::LoadFromOBJ(path);
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 0.5f, 0.5f, 0.5f });

	// 半透明設定 (Object3d拡張前提)
	object3d_->SetColor({ 1.0f, 1.0f, 1.0f, 0.5f });
}

void Wind::Update() {
	if (object3d_) object3d_->Update();
}

void Wind::Draw() {
	if (object3d_) object3d_->Draw();
}

Vector3 Wind::GetWorldPosition() const {
	if (object3d_) return object3d_->GetTranslate();
	return { 0,0,0 };
}

AABB Wind::GetAABB() {
	Vector3 pos = GetWorldPosition();
	return {
		{ pos.x - range_.x / 2.0f, pos.y - range_.y / 2.0f, pos.z - range_.z / 2.0f },
		{ pos.x + range_.x / 2.0f, pos.y + range_.y / 2.0f, pos.z + range_.z / 2.0f }
	};
}

Vector3 Wind::GetVelocity() const {
	return { direction_.x * strength_, direction_.y * strength_, direction_.z * strength_ };
}