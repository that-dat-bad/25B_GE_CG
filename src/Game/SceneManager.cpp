#include "SceneManager.h"
#include "TDEngine.h" // エンジン機能へのアクセス

// 各シーンのヘッダー
#include "TitleScene.h"
#include "TutorialScene.h"
#include "GameScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"

SceneManager::~SceneManager() {
	if (title_) delete title_;
	if (tutorial_) delete tutorial_;
	if (game_) delete game_;
	if (clear_) delete clear_;
	if (gameover_) delete gameover_;
}

void SceneManager::Initialize() {
	// 最初のシーンをタイトルに設定
	scene_ = Scene::kTitle;
	title_ = new TitleScene();
	title_->Initialize();
}

void SceneManager::Update() {
	// シーン遷移チェック
	ChangeScene();

	// 現在のシーンの更新
	switch (scene_) {
	case Scene::kTitle:
		if (title_) title_->Update();
		break;
	case Scene::kTutorial:
		if (tutorial_) tutorial_->Update();
		break;
	case Scene::kGame:
		if (game_) game_->Update();
		break;
	case Scene::kClear:
		if (clear_) clear_->Update();
		break;
	case Scene::kGameOver:
		if (gameover_) gameover_->Update();
		break;
	}
}

void SceneManager::Draw() {
	// 描画処理
	// TDEngineでは描画コマンドの積み込みを行う
	switch (scene_) {
	case Scene::kTitle:
		if (title_) title_->Draw();
		break;
	case Scene::kTutorial:
		if (tutorial_) tutorial_->Draw();
		break;
	case Scene::kGame:
		if (game_) game_->Draw();
		break;
	case Scene::kClear:
		if (clear_) clear_->Draw();
		break;
	case Scene::kGameOver:
		if (gameover_) gameover_->Draw();
		break;
	}
}

void SceneManager::ChangeScene() {
	switch (scene_) {
	case Scene::kTitle:
		if (title_->IsFinished()) {
			// タイトルの選択結果に応じて分岐
			if (title_->GetSelect() == TitleScene::Select::kGame) {
				scene_ = Scene::kGame;
				delete title_;
				title_ = nullptr;

				game_ = new GameScene();
				game_->Initialize();
			}
			else if (title_->GetSelect() == TitleScene::Select::kTutorial) {
				scene_ = Scene::kTutorial;
				delete title_;
				title_ = nullptr;

				tutorial_ = new TutorialScene();
				tutorial_->Initialize();
			}
		}
		break;

	case Scene::kTutorial:
		if (tutorial_->IsFinished()) {
			scene_ = Scene::kGame;
			delete tutorial_;
			tutorial_ = nullptr;

			game_ = new GameScene();
			game_->Initialize();
		}
		break;

	case Scene::kGame:
		if (game_->isFinished()) {
			if (game_->IsClear()) {
				scene_ = Scene::kClear;
				delete game_;
				game_ = nullptr;

				clear_ = new ClearScene();
				clear_->Initialize();
			}
			else if (game_->IsGameover()) {
				scene_ = Scene::kGameOver;
				delete game_;
				game_ = nullptr;

				gameover_ = new GameOverScene();
				gameover_->Initialize();
			}
		}
		break;

	case Scene::kClear:
		if (clear_->IsFinished()) {
			scene_ = Scene::kTitle;
			delete clear_;
			clear_ = nullptr;

			title_ = new TitleScene();
			title_->Initialize();
		}
		break;

	case Scene::kGameOver:
		if (gameover_->IsFinished()) {
			scene_ = Scene::kTitle;
			delete gameover_;
			gameover_ = nullptr;

			title_ = new TitleScene();
			title_->Initialize();
		}
		break;
	}
}