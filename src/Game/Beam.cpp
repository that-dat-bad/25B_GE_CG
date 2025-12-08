#include "Beam.h"
#include "Model.h"

using namespace MyMath;

Beam::~Beam() {
	if (object3d_) delete object3d_;
}

void Beam::Initialize(const Vector3& position) {
	std::string path = "./Resources/beam/beam.obj";
	Model::LoadFromOBJ(path);
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });
}

void Beam::Update() {
	// スケール変更
	Vector3 currentScale = object3d_->GetScale();
	object3d_->SetScale(Add(currentScale, upScale_));

	// 位置変更
	Vector3 currentPos = object3d_->GetTranslate();
	object3d_->SetTranslate(Add(currentPos, beamVelocity_));

	// 当たり判定サイズ調整
	width_ += (upScale_.x * 1.6f);

	object3d_->Update();
}

void Beam::Draw() {
	if (object3d_) object3d_->Draw();
}

Vector3 Beam::GetWorldPosition() const {
	if (object3d_) return object3d_->GetTranslate();
	return { 0, 0, 0 };
}

AABB Beam::GetAABB() {
	Vector3 pos = GetWorldPosition();
	return {
		{ pos.x - width_ / 2.0f, pos.y - height_ / 2.0f, pos.z - width_ / 2.0f },
		{ pos.x + width_ / 2.0f, pos.y + height_ / 2.0f, pos.z + width_ / 2.0f }
	};
}