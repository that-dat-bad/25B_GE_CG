#pragma once

// 各シーンのヘッダー（これらもGameScene同様に移植が必要です）
class TitleScene;
class TutorialScene;
class GameScene;
class ClearScene;
class GameOverScene;

class SceneManager {
public:
	// シーンの種類
	enum class Scene {
		kUnknown = 0,
		kTitle,    // タイトル
		kTutorial, // チュートリアル
		kGame,     // ゲームプレイ
		kClear,    // クリア
		kGameOver, // ゲームオーバー
	};

	~SceneManager();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

private:
	// シーン遷移ロジック
	void ChangeScene();

	// 現在のシーン
	Scene scene_ = Scene::kUnknown;

	// 各シーンのインスタンス
	TitleScene* title_ = nullptr;
	TutorialScene* tutorial_ = nullptr;
	GameScene* game_ = nullptr;
	ClearScene* clear_ = nullptr;
	GameOverScene* gameover_ = nullptr;
};