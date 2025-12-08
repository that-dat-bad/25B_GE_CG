#include "DeathEx.h"
#include "Model.h"
using namespace MyMath;

DeathEx::~DeathEx() { if (object3d_) delete object3d_; }

void DeathEx::Initialize(const Vector3& position, const Vector3& rotate) {
	std::string path = "./Resources/deathEx/deathEx.obj";
	Model::LoadFromOBJ(path);
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);
	object3d_->SetTranslate(position);
	object3d_->SetRotate(rotate);
	object3d_->SetScale({ 70.0f, 2.0f, 1.0f });

	color_ = { 1.0f, 1.0f, 1.0f, 0.0f };
	object3d_->SetColor(color_);
}

void DeathEx::Update() {
	t_ += 0.05f;

	if (!isAlphaMax_) {
		if (color_.w >= 1.0f) isAlphaMax_ = true;
	}
	else {
		return; // Maxになったら何もしない(元コード通り)
	}

	// アルファ値のLerp (LerpFloatの代わり)
	color_.w = Lerp(color_.w, targetAlpha_, t_);
	object3d_->SetColor(color_);

	object3d_->Update();
}

void DeathEx::Draw() {
	if (object3d_) object3d_->Draw();
}