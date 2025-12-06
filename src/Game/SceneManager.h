#pragma once

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

public:
	~SceneManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	// 現在のシーン
	Scene scene_ = Scene::kUnknown;

	// タイトルシーン
	TitleScene* title_ = nullptr;
	//チュートリアルシーン
	TutorialScene* tutorial_ = nullptr;
	// ゲームシーン
	GameScene* game_ = nullptr;
	// クリア
	ClearScene* clear_ = nullptr;
	// ゲームオーバー
	GameOverScene* gameover_ = nullptr;

	private:
	/// <summary>
	/// シーンの遷移
	/// </summary>
	void ChangeScene();
};
