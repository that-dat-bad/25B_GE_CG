#include "Needle.h"

using namespace MyMath;

Needle::~Needle() { if (object3d_) delete object3d_; }

void Needle::Initialize(const Vector3& position, const Vector3& rotate) {
	object3d_ = Object3d::Create();
	object3d_->SetModel("needle.obj");
	object3d_->SetTranslate(position);
	object3d_->SetRotate(rotate);
}

void Needle::Update() {
	t_ += 0.01f;
	// Scaleをイージング
	Vector3 currentScale = object3d_->GetScale();
	Vector3 newScale = EaseIn(t_, currentScale, upScale_);
	object3d_->SetScale(newScale);
	object3d_->Update();
}

void Needle::Draw() { if (object3d_) object3d_->Draw(); }

Vector3 Needle::GetWorldPosition() const {
	return object3d_ ? object3d_->GetTranslate() : Vector3{ 0,0,0 };
}

OBB Needle::GetOBB() {
	OBB obb;
	if (!object3d_) return obb;

	obb.center = GetWorldPosition();
	Vector3 scale = object3d_->GetScale();
	obb.size = { width_ * scale.x, height_ * scale.y, width_ * scale.z };

	// 回転行列を作成して軸を取得
	// MyMath::MakeRotateMatrix を使用
	Matrix4x4 matRot = MakeRotateMatrix(object3d_->GetRotate());

	// 行列の各列が軸ベクトル(X,Y,Z)に対応
	obb.orientations[0] = { matRot.m[0][0], matRot.m[0][1], matRot.m[0][2] }; // X
	obb.orientations[1] = { matRot.m[1][0], matRot.m[1][1], matRot.m[1][2] }; // Y
	obb.orientations[2] = { matRot.m[2][0], matRot.m[2][1], matRot.m[2][2] }; // Z

	return obb;
}