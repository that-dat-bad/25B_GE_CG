#pragma once
#include "IScene.h"
#include <list>
#include <optional>
#include <string>
#include <vector>
#include "ModelManager.h"

#include "stage/Enemy.h"
#include "stage/Explosion.h"
#include "stage/Ground.h"
#include "stage/Player.h"
#include "stage/Reticle.h"
// 敵出現データ
struct EnemySpawnData {
	int wave;      // ウェーブ数
	int spawnTime; // ウェーブ開始からの経過時間
	Vector3 position;
	Vector3 velocity;
	std::string type;
	std::string attackPattern;
};

// ウェーブの状態
enum class WaveState {
	Intro,  // "Wave X... Ready... Start!" などの演出中
	Battle, // 戦闘中（敵が出現・交戦）
	Clear,  // ウェーブクリア後の待機時間
};

class StageScene : public IScene {
public:
	~StageScene() override;
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;
	void Finalize() override;

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
	Model* playerModel_ = nullptr;
	Model* playerBulletModel_ = nullptr;
	Model* enemyModel_ = nullptr;
	Model* enemyBulletModel_ = nullptr;
	Model* enemyMissileModel_ = nullptr;
	Model* playerMissileModel_ = nullptr;
	Model* explosionModel_ = nullptr;
	Model* groundModel_ = nullptr;

	// クラス・オブジェクト
	Camera camera_;
	Player* player_ = nullptr;
	Ground* ground_ = nullptr;
	// WorldTransform worldTransform_; // 削除
	Input* input_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	// ゲームオブジェクトリスト
	std::list<Enemy*> enemies_;
	std::list<EnemySpawnData> enemySpawnList_;
	std::list<Explosion*> explosions_;

	// レティクル・ロックオン
	Reticle* reticle_ = nullptr;
	Enemy* lockedEnemy_ = nullptr;
	Sprite* lockOnMark_ = nullptr;

	// UIスプライト
	Sprite* hpBarSprite_ = nullptr;
	Sprite* lifeIconSprite_ = nullptr;
	Sprite* fadeSprite_ = nullptr;

	// ★追加: 演出用スプライト
	Sprite* spriteWave_ = nullptr;
	Sprite* spriteReady_ = nullptr;
	Sprite* spriteStart_ = nullptr;
	Sprite* spriteClear_ = nullptr;

	// ゲームステータス
	int score_ = 0;
	int32_t gameLimitTimer_ = 0; // 全体の制限時間（必要なら）

	// ウェーブ管理
	int currentWave_ = 1;
	WaveState waveState_ = WaveState::Intro;
	int waveTimer_ = 0; // 演出やスポーン用の汎用タイマー
};