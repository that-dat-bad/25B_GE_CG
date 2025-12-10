#pragma once
#include "Math/MyMath.h"
#include <vector>
#include <list>
#include "AudioManager.h"
#include "Sprite.h"

// 前方宣言
class Player;
class Enemy;
class ChainBomb;
class Wind;
class Skydome;
class TimeLimit;
class Fade;
class EnemyHPGauge;

class GameScene {
public:
	// デストラクタ
	~GameScene();

	// 初期化処理
	void Initialize();

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// 全ての当たり判定を行う
	void CheckAllCollisions();

public:
	// シーンの終了フラグのGetter
	bool isFinished() const { return isFinished_; }
	bool IsClear() const { return isClear_; }
	bool IsGameover() const { return isGameover_; }

private:
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン
		kFadeOut, // フェードアウト
	};

	Phase phase_ = Phase::kMain;

private:
	// TDEngineではカメラは CameraManager で管理するため、ここでは保持しません
	// 必要に応じてカメラの操作用のIDやポインタを持つことは可能です

	// 各ゲームオブジェクト
	Player* player_ = nullptr;
	const MyMath::Vector3 playerPos_ = { -27.0f, 0.0f, 0.0f };

	Enemy* enemy_ = nullptr;
	MyMath::Vector3 enemyPos_ = { 0.0f, 20.0f, 0.0f };

	EnemyHPGauge *hpGauge_ = nullptr;

	std::vector<ChainBomb*> chainBombs_;

	Wind* wind_ = nullptr;
	MyMath::Vector3 windPos_ = { 12.4f, -10.0f, 0.0f };

	Skydome* skydome_ = nullptr;

	// UI・システム
	TimeLimit* timeLimit_ = nullptr;
	Fade* fade_ = nullptr;
	float duration_ = 0.5f;

	// シーンの終了フラグ
	bool isFinished_ = false;
	bool isClear_ = false;
	bool isGameover_ = false;

	  Sprite *operation_ = nullptr;


	// SE
	SoundData soundBgm_;
	IXAudio2SourceVoice* pBgmVoice_ = nullptr;

private:
	// フェードインの更新
	void UpdateFadeIn();
	// メインの更新
	void UpdateMain();
	// フェードアウトの更新
	void UpdateFadeOut();
};