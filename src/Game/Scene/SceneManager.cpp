#include "SceneManager.h"
#include "TitleScene.h"
#include "StageScene.h"
#include "ClearScene.h"
#include "DebugScene.h"
#include "IScene.h"

SceneManager::SceneManager() {
	// 初期シーン生成 (開発中は DebugScene から起動)
	currentScene = std::make_unique<DebugScene>();
	currentScene->Initialize();
	currentSceneID = SCENE::DEBUG;
}

SceneManager::~SceneManager() {
}

void SceneManager::Update() {

	// 現在のシーンの更新
	if (currentScene != nullptr) {
		currentScene->Update();
	}

	// シーン切り替え判定（static変数が変更されたかチェック）
	int nextSceneID = currentScene->GetSceneID();
	if (nextSceneID != currentSceneID) {

		switch (nextSceneID) {
		case SCENE::TITLE:
			currentScene = std::make_unique<TitleScene>();
			break;
		case SCENE::STAGE:
			currentScene = std::make_unique<StageScene>();
			break;
		case SCENE::CLEAR:
			currentScene = std::make_unique<ClearScene>();
			break;
		case SCENE::DEBUG:
			currentScene = std::make_unique<DebugScene>();
			break;
		default:
			currentScene = nullptr;
			break;
		}

		if (currentScene != nullptr) {
			currentScene->Initialize();
		}

		currentSceneID = nextSceneID;
	}
}

void SceneManager::Draw() {
	if (currentScene != nullptr) {
		currentScene->Draw();
	}
}