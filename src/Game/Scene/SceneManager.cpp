#include "SceneManager.h"
#include "TitleScene.h"
#include "StageScene.h"
#include "ClearScene.h"
#include "IScene.h"

SceneManager::SceneManager() {
	currentSceneID_ = SceneID::kTitle;
	currentScene_ = std::make_unique<TitleScene>();
	currentScene_->Initialize();
}

SceneManager::~SceneManager() {
}

void SceneManager::Update() {

	if (currentScene_ == nullptr) {
		return;
	}

	std::optional<SceneID> nextSceneID = currentScene_->Update();

	if (nextSceneID.has_value()) {
		SceneID next = nextSceneID.value();
		
		if (next != currentSceneID_) {
			currentScene_->Finalize();

			switch (next) {
			case SceneID::kTitle:
				currentScene_ = std::make_unique<TitleScene>();
				break;
			case SceneID::kStage:
				currentScene_ = std::make_unique<StageScene>();
				break;
			case SceneID::kClear:
				currentScene_ = std::make_unique<ClearScene>();
				break;
			}

			if (currentScene_) {
				currentScene_->Initialize();
			}
			currentSceneID_ = next;
		}
	}
}

void SceneManager::Draw() {
	if (currentScene_ != nullptr) {
		currentScene_->Draw();
	}
}
