#include "Thunder.h"
#include <algorithm>
#include "Model.h"

using namespace MyMath;

Thunder::~Thunder() {
	if (object3d_) delete object3d_;
}

void Thunder::Initialize(const Vector3& position) {
	std::string path = "./Resources/Thunder/Thunder.obj";
	Model::LoadFromOBJ(path);
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);
	object3d_->SetTranslate(position);
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void Thunder::Update() {
	t_ += 0.01f;

	if (isAlphaMax_) {
		if (color_.w <= 0.5f) isCollisionDisabled_ = true;
		if (t_ >= 1.0f) return;
	}
	else {
		if (color_.w >= 1.0f) {
			isAlphaMax_ = true;
			targetAlpha_ = 0.0f;
			t_ = 0.0f;
		}
	}

	// 補間 (MyMathに追加したEaseOut等を使用)
	color_.w = EaseOut(t_, color_.w, targetAlpha_);
	object3d_->SetColor(color_);
	object3d_->Update();
}

void Thunder::Draw() {
	if (object3d_) object3d_->Draw();
}

Vector3 Thunder::GetWorldPosition() const {
	if (object3d_) return object3d_->GetTranslate();
	return { 0,0,0 };
}

AABB Thunder::GetAABB() {
	Vector3 pos = GetWorldPosition();
	return {
		{ pos.x - width_ / 2.0f, pos.y - height_ / 2.0f, pos.z - width_ / 2.0f },
		{ pos.x + width_ / 2.0f, pos.y + height_ / 2.0f, pos.z + width_ / 2.0f }
	};
}