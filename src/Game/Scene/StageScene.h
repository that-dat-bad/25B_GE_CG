#pragma once
#include "IScene.h"
#include <list>
#include <optional>
#include <string>
#include <vector>
#include "ModelManager.h"
#include "AudioManager.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "Camera.h"

class Input;
class DebugCamera;

#include "Stage/Enemy.h"
#include "Stage/Explosion.h"
#include "Stage/Ground.h"
#include "Stage/Player.h"
#include "Stage/Reticle.h"
struct EnemySpawnData {
	int wave;      
	int spawnTime; 
	Vector3 position;
	Vector3 velocity;
	std::string type;
	std::string attackPattern;
};

enum class WaveState {
	Intro,  
	Battle, 
	Clear,  
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
	uint32_t textureHandle_ = 0;
	uint32_t uiTexHandle_ = 0;
	uint32_t lockOnTex_ = 0;
	uint32_t fadeTextureHandle_ = 0;

	uint32_t texWave_ = 0;
	uint32_t texReady_ = 0;
	uint32_t texStart_ = 0;
	uint32_t texClear_ = 0;

	SoundData soundLockOn_{};
	SoundData soundMissile_{};
	SoundData soundExplosion_{};
	bool isLockSoundPlayed_ = false;

	Model* playerModel_ = nullptr;
	Model* playerBulletModel_ = nullptr;
	Model* enemyModel_ = nullptr;
	Model* enemyBulletModel_ = nullptr;
	Model* enemyMissileModel_ = nullptr;
	Model* playerMissileModel_ = nullptr;
	Model* explosionModel_ = nullptr;
	Model* groundModel_ = nullptr;

	Camera camera_;
	Player* player_ = nullptr;
	Ground* ground_ = nullptr;
	Input* input_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	std::list<Enemy*> enemies_;
	std::list<EnemySpawnData> enemySpawnList_;
	std::list<Explosion*> explosions_;

	Reticle* reticle_ = nullptr;
	Enemy* lockedEnemy_ = nullptr;
	Sprite* lockOnMark_ = nullptr;

	Sprite* hpBarSprite_ = nullptr;
	Sprite* lifeIconSprite_ = nullptr;
	Sprite* fadeSprite_ = nullptr;

	Sprite* spriteWave_ = nullptr;
	Sprite* spriteReady_ = nullptr;
	Sprite* spriteStart_ = nullptr;
	Sprite* spriteClear_ = nullptr;

	int score_ = 0;
	int32_t gameLimitTimer_ = 0; 

	int currentWave_ = 1;
	WaveState waveState_ = WaveState::Intro;
	int waveTimer_ = 0; 
};
