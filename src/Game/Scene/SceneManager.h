#pragma once
#include "IScene.h"
#include <memory> 

/// <summary>
/// シーンの遷移やライフサイクルを管理するクラス
/// </summary>
class SceneManager {
private:
	std::unique_ptr<IScene> currentScene = nullptr;
	int currentSceneID; // 現在管理しているシーンIDを保持

public:
	/// <summary>コンストラクタ（初期シーンの生成を行う）</summary>
	SceneManager();
	~SceneManager();

	/// <summary>
	/// 現在のシーンの更新と、必要に応じたシーン遷移処理を行う
	/// </summary>
	void Update();

	/// <summary>
	/// 現在のシーンの描画処理を行う
	/// </summary>
	void Draw();
};