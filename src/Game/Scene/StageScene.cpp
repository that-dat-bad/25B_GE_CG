#include "CameraManager.h"
#include "StageScene.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "SpriteCommon.h"

#include <cmath>
#include <ModelManager.h>

void StageScene::Initialize() {
	sceneID = SCENE::STAGE;

	// --- 3Dオブジェクト ---
	// --- 3Dオブジェクト ---
	sphereObject = std::make_unique<Object3d>();
	sphereObject->Initialize(Object3dCommon::GetInstance());
	sphereObject->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	ModelManager::GetInstance()->LoadModel("models/sphere.obj");
	sphereObject->SetModel("models/sphere.obj");

	// --- スプライト ---
	sprite_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("assets/textures/uvChecker.png");
	sprite_->Initialize(SpriteCommon::GetInstance(), "assets/textures/uvChecker.png");
	sprite_->SetPosition({ 0.0f, 360.0f });

	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 0.0f, 0.0f, -10.0f });
	CameraManager::GetInstance()->Update();
}

void StageScene::Update() {

	sphereObject->Update();

	sprite_->Update();
}

void StageScene::Draw() {
	// 1. 3D描画
	Object3dCommon::GetInstance()->SetupCommonState();
	sphereObject->Draw();

	// 2. スプライト描画
	SpriteCommon::GetInstance()->SetupCommonState();
	sprite_->Draw();
}

void StageScene::Finalize() {
}