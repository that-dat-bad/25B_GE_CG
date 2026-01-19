#include "CameraManager.h"
#include "StageScene.h"
#include "Object3dCommon.h"
#include <cmath>

void StageScene::Initialize() {
	sceneID = SCENE::STAGE;
	Object3dCommon* common = Object3dCommon::GetInstance();

	sphereObject = new Object3d();
	sphereObject->Initialize(common);
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
