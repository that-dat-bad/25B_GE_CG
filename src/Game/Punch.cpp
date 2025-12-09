#include "Punch.h"
#include <algorithm>
#include "Model.h"
using namespace MyMath;

Punch::~Punch() {
	if (object3d_) delete object3d_;
}

void Punch::Initialize(const Vector3& position, int punched) {
	std::string path = "./Resources/punch/punch.obj";
	Model::LoadFromOBJ(path);
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });

	object3d_->Update();

	// フラグを設定 (int -> bool)
	isPunched_ = (punched != 0);

	// パンチした後の位置を設定 (元コードのロジックを踏襲)
	// 元: punchedPosition = {position.x - 8.0f, position.y, position.z};
	punchedPosition_ = { position.x - 8.0f, position.y, position.z };
}

void Punch::Update() {
	// tの値を増加
	t_ += 0.05f;

	if (t_ >= 1.0f) {
		// tの値をリセット
		t_ = 0.0f;

		if (isPunched_) {
			// パンチが完了したらフラグを戻す (戻る動作へ)
			isPunched_ = false;
			punchedPosition_.x += 6.0f;
		}
		else {
			// パンチが完了したらフラグを立てる (突き出す動作へ)
			isPunched_ = true;
			punchedPosition_.x -= 8.0f;
		}
	}
	else {
		Vector3 currentPos = object3d_->GetTranslate();
		Vector3 nextPos;

		if (isPunched_) {
			// パンチを引っ込める (EaseIn)
			// MyMath::EaseIn は (t, start, end)
			nextPos = EaseIn(t_, currentPos, punchedPosition_);
		}
		else {
			// パンチする (EaseOut)
			nextPos = EaseOut(t_, currentPos, punchedPosition_);
		}

		object3d_->SetTranslate(nextPos);
	}

	// 行列更新
	if (object3d_) object3d_->Update();
}

void Punch::Draw() {
	if (object3d_) object3d_->Draw();
}

void Punch::SetPosition(const Vector3& position) {
	if (object3d_) object3d_->SetTranslate(position);
}

Vector3 Punch::GetWorldPosition() const {
	if (object3d_) return object3d_->GetTranslate();
	return { 0, 0, 0 };
}

AABB Punch::GetAABB() {
	Vector3 pos = GetWorldPosition();
	return {
		{ pos.x - width_ / 2.0f, pos.y - height_ / 2.0f, pos.z - width_ / 2.0f },
		{ pos.x + width_ / 2.0f, pos.y + height_ / 2.0f, pos.z + width_ / 2.0f }
	};
}