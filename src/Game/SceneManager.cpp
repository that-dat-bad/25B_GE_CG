#include "SceneManager.h"
#include "ClearScene.h"
#include "GameOverScene.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "TutorialScene.h"

SceneManager::~SceneManager() {
	// タイトル
	delete title_;
	title_ = nullptr;

	// チュートリアル
	delete tutorial_;
	tutorial_ = nullptr;

	// ゲーム
	delete game_;
	game_ = nullptr;

	// クリア
	delete clear_;
	clear_ = nullptr;

	// ゲームオーバー
	delete gameover_;
	gameover_ = nullptr;
}

/// <summary>
/// 初期化
/// </summary>
void SceneManager::Initialize() {

	// 最初のシーン（タイトル）
	scene_ = Scene::kTitle;
	title_ = new TitleScene;
	title_->Initialize();
}
/// <summary>
/// 更新
/// </summary>
void SceneManager::Update() {

	// シーンの遷移
	ChangeScene();

	switch (scene_) {
	case Scene::kTitle:
		title_->Update();

		break;

	case Scene::kTutorial:
		tutorial_->Update();

		break;
	case Scene::kGame:
		game_->Update();

		break;

	case Scene::kClear:
		clear_->Update();

		break;

	case Scene::kGameOver:
		gameover_->Update();

		break;
	}
}
/// <summary>
/// 描画
/// </summary>
void SceneManager::Draw() {
	switch (scene_) {
	case Scene::kTitle:
		title_->Draw();

		break;
	case Scene::kTutorial:
		tutorial_->Draw();

		break;
	case Scene::kGame:
		game_->Draw();

		break;

	case Scene::kClear:
		clear_->Draw();

		break;

	case Scene::kGameOver:
		gameover_->Draw();

		break;
	}
}

/// <summary>
/// シーンの遷移
/// </summary>
void SceneManager::ChangeScene() {

	switch (scene_) {
	case Scene::kTitle:
		if (title_->IsFinished()) {
			if (title_->GetSelect() == TitleScene::Select::kGame) {
				// シーン変更
				scene_ = Scene::kGame;

				// 旧シーン（タイトル）の解放
				delete title_;
				title_ = nullptr;

				// 新シーン（ゲームプレイ）の生成と初期化
				game_ = new GameScene;
				game_->Initialize();
			} else if (title_->GetSelect() == TitleScene::Select::kTutorial) {
				// シーン変更
				scene_ = Scene::kTutorial;

				// 旧シーン（タイトル）の解放
				delete title_;
				title_ = nullptr;

				// 新シーン（チュートリアル）の生成と初期化
				tutorial_ = new TutorialScene;
				tutorial_->Initialize();
			}
		}

		break;
	case Scene::kTutorial:
		if (tutorial_->IsFinished()) {
			// シーン変更
			scene_ = Scene::kGame;

			// 旧シーン（チュートリアル）の解放
			delete tutorial_;
			tutorial_ = nullptr;

			// 新シーン（ゲームプレイ）の生成と初期化
			game_ = new GameScene;
			game_->Initialize();
		}

		break;

	case Scene::kGame:
		if (game_->isFinished()) {
			
			if (game_->IsClear()) {
				// シーン変更
				scene_ = Scene::kClear;

				// 旧シーン(ゲームシーン)の解放
				delete game_;
				game_ = nullptr;

				// 新シーン(クリア)の生成と初期化
				clear_ = new ClearScene();
				clear_->Initialize();

			} else if (game_->IsGameover()) {
				// シーン変更
				scene_ = Scene::kGameOver;

				// 旧シーン(ゲームシーン)の解放
				delete game_;
				game_ = nullptr;

				// 新シーン(ゲームオーバー)の生成と初期化
				gameover_ = new GameOverScene();
				gameover_->Initialize();
			}
		}

		break;

	case Scene::kClear:
		if (clear_->IsFinished())
		{
			// シーン変更
			scene_ = Scene::kTitle;
			// 旧シーンの解放
			delete clear_;
			clear_ = nullptr;
			// 新シーンの生成と初期化
			title_ = new TitleScene();
			title_->Initialize();
		}

		break;

	case Scene::kGameOver:
		if (gameover_->IsFinished())
		{
			// シーン変更
			scene_ = Scene::kTitle;
			// 旧シーンの解放
			delete gameover_;
			gameover_ = nullptr;
			// 新シーンの生成と初期化
			title_ = new TitleScene();
			title_->Initialize();
		}

		break;
	}
}
