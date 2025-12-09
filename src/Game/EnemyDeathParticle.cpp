#include "EnemyDeathParticle.h"
#include "Model.h"
using namespace MyMath;

EnemyDeathParticle::~EnemyDeathParticle() { if (object3d_) delete object3d_; }

void EnemyDeathParticle::Initialize(const Vector3& position) {
	std::string path = "./Resources/deathParticle/deathParticle.obj";
	Model::LoadFromOBJ(path);
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);
    object3d_->SetTranslate(position);
    object3d_->SetScale(Vector3{8.0f, 8.0f, 8.0f});

	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	object3d_->SetColor(color_);

	object3d_->Update();
}

void EnemyDeathParticle::Update() {
	t_ += 0.01f;

	if (isAlphaMax_) {
		if (t_ >= 1.0f) return;
	}
	else {
		if (color_.w >= 1.0f) {
			isAlphaMax_ = true;
			targetAlpha_ = 0.0f; // フェードアウトへ
			t_ = 0.0f;
		}
	}

	color_.w = EaseOut(t_, color_.w, targetAlpha_);
	object3d_->SetColor(color_);

	object3d_->Update();
}

void EnemyDeathParticle::Draw() {
	if (object3d_) object3d_->Draw();
}