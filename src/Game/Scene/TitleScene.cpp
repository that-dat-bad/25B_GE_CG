#include <dinput.h>
#include "TitleScene.h"
#include "Object3dCommon.h"
#include "ModelManager.h"

void TitleScene::Initialize() {
	// カメラの設定
	camera_.SetTranslate({ 0.0f, 1.0f, -5.0f });
	camera_.SetRotate({ 0.0f, 0.0f, 0.0f });
	camera_.Update();

	// デフォルトカメラとして登録
	Object3dCommon::GetInstance()->SetDefaultCamera(&camera_);

	// Ruruko モデルの読み込み
	ModelManager::GetInstance()->LoadModel("Resources/Ruruko.fbx");

	// Object3d の作成・モデルセット
	rurukoObject_ = std::make_unique<Object3d>();
	rurukoObject_->Initialize(Object3dCommon::GetInstance());
	rurukoObject_->SetModel(ModelManager::GetInstance()->FindModel("Resources/Ruruko.fbx"));
	rurukoObject_->SetScale({ 1.0f, 1.0f, 1.0f });
	rurukoObject_->SetTranslate({ 0.0f, 0.0f, 0.0f });
	rurukoObject_->Update();
}

void TitleScene::Update() {
	// SPACE でステージへ遷移
	if (IScene::IsKeyTriggered(DIK_SPACE)) {
		sceneID = SCENE::STAGE;
	}

	camera_.Update();
	rurukoObject_->Update();
}

void TitleScene::Draw() {
	// 3Dオブジェクト描画
	Object3dCommon::GetInstance()->SetupCommonState();

	if (rurukoObject_) {
		rurukoObject_->Draw();
	}
}

void TitleScene::Finalize() {
}
