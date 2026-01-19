#include "StageScene.h"
#include <cmath>

void StageScene::Initialize() {


	sceneID = SCENE::STAGE;
	sphereObject = new Object3d();
	sphereObject->Initialize(object3dCommon);
	sphereObject->SetModel("models/sphere.obj");
}

void StageScene::Update() {

	sphereObject->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	sphereObject->Update();
}

void StageScene::Draw() {
	sphereObject->Draw();
}
void StageScene::Finalize() {
	delete sphereObject;
}
