#include "TitleLogo.h"
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Object3dCommon.h"

using namespace MyMath;

void TitleLogo::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// nullポインタチェック
	assert(model);

	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(model);
	object3d_->SetCamera(camera);
	object3d_->SetTranslate(position);
	
	object3d_->Update();
}

void TitleLogo::Update() {
	if(object3d_) object3d_->Update();
}

void TitleLogo::Draw() {
	if(object3d_) object3d_->Draw();
}

void TitleLogo::SetPosition(const Vector3& position) {
	if(object3d_) object3d_->SetTranslate(position);
}