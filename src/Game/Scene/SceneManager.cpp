#include "SceneManager.h"
#include "TitleScene.h"
#include "StageScene.h"
#include "ClearScene.h"
#include "IScene.h"

SceneManager::SceneManager() {
	// 初期シーン生成 (タイトルから始めるのが一般的だが、デバッグならStageからでも可。ここではタイトルにする)
	currentSceneID_ = SceneID::kTitle;
	currentScene_ = std::make_unique<TitleScene>();
	currentScene_->Initialize();
}

SceneManager::~SceneManager() {
}

void SceneManager::Update() {

	// 現在のシーンの更新
	if (currentScene_ == nullptr) {
		return;
	}

	std::optional<SceneID> nextSceneID = currentScene_->Update();

	// シーン遷移リクエストがある場合
	if (nextSceneID.has_value()) {
		SceneID next = nextSceneID.value();
		
		// 現在のシーンと異なれば切り替え（あるいは同シーンのリセットも可）
		if (next != currentSceneID_) {
			// 現在のシーンの終了処理
			currentScene_->Finalize();

			// 次のシーンの生成
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

			// 新しいシーンの初期化
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