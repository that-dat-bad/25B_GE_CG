#include "SceneManager.h"
#include "TitleScene.h"
#include "StageScene.h"
#include "ClearScene.h"
#include "ResultScene.h"
#include "DebugScene.h"
#include "IScene.h"

#ifdef USE_IMGUI
#include "../../../external/imgui/imgui.h"
#endif

SceneManager::SceneManager() {
	// 初期シーン生成
	currentScene = std::make_unique<TitleScene>();
	currentScene->Initialize();
	currentSceneID = SCENE::TITLE;
	IScene::SetSceneID(SCENE::TITLE);
}

SceneManager::~SceneManager() {
}

void SceneManager::Update() {

	// 現在のシーンの更新
	if (currentScene != nullptr) {
		currentScene->Update();
	}

#ifdef USE_IMGUI
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Scene")) {
			if (ImGui::MenuItem("Title")) {
				IScene::SetSceneID(SCENE::TITLE);
			}
			if (ImGui::MenuItem("Stage")) {
				IScene::SetSceneID(SCENE::STAGE);
			}
			if (ImGui::MenuItem("Result")) {
				IScene::SetSceneID(SCENE::RESULT);
			}
			if (ImGui::MenuItem("Clear")) {
				IScene::SetSceneID(SCENE::CLEAR);
			}
			if (ImGui::MenuItem("Debug")) {
				IScene::SetSceneID(SCENE::DEBUG);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
#endif

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
		case SCENE::RESULT:
			currentScene = std::make_unique<ResultScene>();
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