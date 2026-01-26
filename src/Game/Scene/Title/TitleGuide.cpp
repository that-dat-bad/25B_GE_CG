#include "TitleGuide.h"

#include <cassert>
#include <cmath>
#include "Object3dCommon.h"

using namespace MyMath;

void TitleGuide::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// nullポインタチェック
	assert(model);

	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(model);
	object3d_->SetCamera(camera);
	object3d_->SetTranslate(position);

	// 点滅用パラメータの初期化
	blinkParameter_ = 0.0f;
}

void TitleGuide::Update() {
	// 時間を進める
	blinkParameter_ += kBlinkSpeed_;

	float alpha = (std::sin(blinkParameter_) + 1.0f) / 2.0f;

	// 色変更でアルファ対応
	object3d_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });

	object3d_->Update();
}


void TitleGuide::Draw() {
	if(object3d_) object3d_->Draw();
}

void TitleGuide::SetPosition(const Vector3& position) {
	if(object3d_) object3d_->SetTranslate(position);
}