#include "SceneManager.h"
#include "TitleScene.h"
#include "StageScene.h"
#include "ClearScene.h"
#include "IScene.h"

SceneManager::SceneManager() {
	// 初期シーン生成
	currentScene = new TitleScene();
	currentScene->Initialize();
	currentSceneID = SCENE::TITLE;
}

SceneManager::~SceneManager() {
	if (currentScene != nullptr) {
		delete currentScene;
	}
}

void SceneManager::Update() {

	// 現在のシーンの更新
	if (currentScene != nullptr) {
		currentScene->Update();
	}

	// シーン切り替え判定（static変数が変更されたかチェック）
	int nextSceneID = currentScene->GetSceneID();
	if (nextSceneID != currentSceneID) {

		delete currentScene;

		switch (nextSceneID) {
		case SCENE::TITLE:
			currentScene = new TitleScene();
			break;
		case SCENE::STAGE:
			currentScene = new StageScene();
			break;
		case SCENE::CLEAR:
			currentScene = new ClearScene();
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