#include "Ground.h"
#include "Math/MyMath.h"
#include "Object3dCommon.h"

void Ground::Initialize(Model* model, Camera* camera) {
	model_ = model;

	object3ds_.resize(kGroundCount);

	for (int i = 0; i < kGroundCount; i++) {
		object3ds_[i] = std::make_unique<Object3d>();
		object3ds_[i]->Initialize(Object3dCommon::GetInstance());
		object3ds_[i]->SetModel(model_);
		object3ds_[i]->SetCamera(camera);

		object3ds_[i]->SetScale({ 1.0f, 1.0f, 1.0f });

		Vector3 pos = object3ds_[i]->GetTranslate();
		pos.y = -10.0f;
		pos.z = static_cast<float>(i) * kGroundDepth;
		object3ds_[i]->SetTranslate(pos);

		object3ds_[i]->Update();
	}
}

void Ground::Update() {
	const float kScrollSpeed = 3.0f;

	for (int i = 0; i < kGroundCount; i++) {
		Vector3 pos = object3ds_[i]->GetTranslate();
		pos.z -= kScrollSpeed;

		if (pos.z <= -kGroundDepth) {
			pos.z += kGroundCount * kGroundDepth;
		}

		object3ds_[i]->SetTranslate(pos);
		object3ds_[i]->Update();
	}
}

void Ground::Draw() {
	for (int i = 0; i < kGroundCount; i++) {
		if(object3ds_[i]) object3ds_[i]->Draw();
	}
}
