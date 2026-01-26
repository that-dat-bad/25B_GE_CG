#pragma once
#include "Enemy.h"
#include "Explosion.h"
#include "Ground.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include "Reticle.h"
#include <list>
#include <optional>
#include <string>
#include <vector>

// 敵出現データ
struct EnemySpawnData {
	int wave;      // ウェーブ数
	int spawnTime; // ウェーブ開始からの経過時間
	KamataEngine::Vector3 position;
	KamataEngine::Vector3 velocity;
	std::string type;
	std::string attackPattern;
};

// ウェーブの状態
enum class WaveState {
	Intro,  // "Wave X... Ready... Start!" などの演出中
	Battle, // 戦闘中（敵が出現・交戦）
	Clear,  // ウェーブクリア後の待機時間
};

class GameScene : public IScene {
public:
	~GameScene() override;
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;

private:
	std::optional<SceneID> UpdateFadeIn();
	std::optional<SceneID> UpdateMain();
	std::optional<SceneID> UpdateFadeOut();

private:
	// テクスチャ
	uint32_t textureHandle_ = 0;
	uint32_t uiTexHandle_ = 0;
	uint32_t lockOnTex_ = 0;
	uint32_t fadeTextureHandle_ = 0;

	// ★追加: 演出用テクスチャ
	uint32_t texWave_ = 0;
	uint32_t texReady_ = 0;
	uint32_t texStart_ = 0;
	uint32_t texClear_ = 0;

	// 音声ハンドル
	uint32_t soundLockOn_ = 0;
	uint32_t soundMissile_ = 0;
	uint32_t soundExplosion_ = 0;
	bool isLockSoundPlayed_ = false;

	// 3Dモデル
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* playerBulletModel_ = nullptr;
	KamataEngine::Model* enemyModel_ = nullptr;
	KamataEngine::Model* enemyBulletModel_ = nullptr;
	KamataEngine::Model* enemyMissileModel_ = nullptr;
	KamataEngine::Model* playerMissileModel_ = nullptr;
	KamataEngine::Model* explosionModel_ = nullptr;
	KamataEngine::Model* groundModel_ = nullptr;

	// クラス・オブジェクト
	KamataEngine::Camera camera_;
	Player* player_ = nullptr;
	Ground* ground_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	// ゲームオブジェクトリスト
	std::list<Enemy*> enemies_;
	std::list<EnemySpawnData> enemySpawnList_;
	std::list<Explosion*> explosions_;

	// レティクル・ロックオン
	Reticle* reticle_ = nullptr;
	Enemy* lockedEnemy_ = nullptr;
	KamataEngine::Sprite* lockOnMark_ = nullptr;

	// UIスプライト
	KamataEngine::Sprite* hpBarSprite_ = nullptr;
	KamataEngine::Sprite* lifeIconSprite_ = nullptr;
	KamataEngine::Sprite* fadeSprite_ = nullptr;

	// ★追加: 演出用スプライト
	KamataEngine::Sprite* spriteWave_ = nullptr;
	KamataEngine::Sprite* spriteReady_ = nullptr;
	KamataEngine::Sprite* spriteStart_ = nullptr;
	KamataEngine::Sprite* spriteClear_ = nullptr;

	// ゲームステータス
	int score_ = 0;
	int32_t gameLimitTimer_ = 0; // 全体の制限時間（必要なら）

	// ウェーブ管理
	int currentWave_ = 1;
	WaveState waveState_ = WaveState::Intro;
	int waveTimer_ = 0; // 演出やスポーン用の汎用タイマー
};