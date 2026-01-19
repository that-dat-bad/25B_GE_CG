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
	Object3dCommon::GetInstance()->SetupCommonState();
	// 定数バッファをセット
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(4, lightingSettingsResource->GetGPUVirtualAddress());

	sphereObject->Draw();
}
void StageScene::Finalize() {
	delete sphereObject;
}
