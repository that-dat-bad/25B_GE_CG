#pragma once
#include "Fade.h"
#include "BackGround.h"
#include <TDEngine.h>

class GameOverScene {
public:
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

public:
	// シーン終了フラグのGetter
	bool IsFinished() const { return isFinished_; }

private:
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン
		kFadeOut, // フェードアウト
	};

	Phase phase_ = Phase::kMain;

private:
	// フェード
	Fade fade_;
	float duration_ = 0.5f;

	// シーン終了フラグ
	bool isFinished_ = false;

	// カメラ
	TDEngine::Camera camera_;
	// 背景
	BackGround* backGround_ = nullptr;
	// 背景の位置
	TDEngine::Vector3 pos = { 0.0f, 0.0f, 0.0f };
	// 背景のモデル
	TDEngine::Model* modelBackground_ = nullptr;

private:
	// フェードインの更新
	void UpdateFadeIn();
	// メインの更新
	void UpdateMain();
	// フェードアウトの更新
	void UpdateFadeOut();
};