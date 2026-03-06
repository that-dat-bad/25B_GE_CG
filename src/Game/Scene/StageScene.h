#pragma once
#include "IScene.h"
#include <list>
#include <memory>
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

enum class PauseMenuItem {
	Resume = 0,
	Restart,
	BackToTitle,
	Count
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
	std::optional<SceneID> UpdatePause();

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
	IXAudio2SourceVoice* voiceLockSound_ = nullptr;
	IXAudio2SourceVoice* voiceMissileSound_ = nullptr;
	PlayerMissile* trackingMissile_ = nullptr;

	Model* playerModel_ = nullptr;
	Model* playerBulletModel_ = nullptr;
	Model* enemyModel_ = nullptr;
	Model* enemyBulletModel_ = nullptr;
	Model* enemyMissileModel_ = nullptr;
	Model* playerMissileModel_ = nullptr;
	Model* explosionModel_ = nullptr;
	Model* groundModel_ = nullptr;
	Model* skydomeModel_ = nullptr;
	std::unique_ptr<Object3d> skydome_;

	Camera camera_;
	std::unique_ptr<Player> player_;
	std::unique_ptr<Ground> ground_;
	Input* input_ = nullptr;
	std::unique_ptr<DebugCamera> debugCamera_;
	bool isDebugCameraActive_ = false;

	std::list<std::unique_ptr<Enemy>> enemies_;
	std::list<EnemySpawnData> enemySpawnList_;
	std::list<std::unique_ptr<Explosion>> explosions_;

	std::unique_ptr<Reticle> reticle_;
	Enemy* lockedEnemy_ = nullptr;
	std::unique_ptr<Sprite> lockOnMark_;

	std::unique_ptr<Sprite> hpBarSprite_;
	std::unique_ptr<Sprite> lifeIconSprite_;
	std::unique_ptr<Sprite> fadeSprite_;

	std::unique_ptr<Sprite> spriteWave_;
	std::unique_ptr<Sprite> spriteReady_;
	std::unique_ptr<Sprite> spriteStart_;
	std::unique_ptr<Sprite> spriteClear_;
	
	// Minimap
	std::unique_ptr<Sprite> minimapBg_;
	std::unique_ptr<Sprite> minimapPlayer_;

	int score_ = 0;
	int32_t gameLimitTimer_ = 0; 

	int currentWave_ = 1;
	WaveState waveState_ = WaveState::Intro;
	int waveTimer_ = 0; 

	// ポーズ関連
	bool isPaused_ = false;
	PauseMenuItem pauseMenuSelection_ = PauseMenuItem::Resume;
	bool requestRestart_ = false;

	std::unique_ptr<Sprite> pauseOverlay_;
	std::unique_ptr<Sprite> pauseMenuBg_;
	std::unique_ptr<Sprite> pauseMenuItems_[3];
};
