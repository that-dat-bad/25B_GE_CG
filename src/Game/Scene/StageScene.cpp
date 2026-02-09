#include "StageScene.h"
#include "Stage/Enemy.h"
#include "Stage/Explosion.h"
#include "Stage/Ground.h"


#include "Stage/Player.h"
#include "Stage/PlayerMissile.h"
#include "Stage/PlayerBullet.h"
#include "ClearScene.h"
#include "Stage/Reticle.h"
#include "json.hpp"
#include "ImguiManager.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include "Input.h"
#include "DebugCamera.h"
#include "Camera.h"
#include "Object3d.h"
#include "Math/MyMath.h"

using namespace MyMath;

#include <assert.h>
#include <cmath>
#include <fstream>
#include <algorithm>

using json = nlohmann::json;
using namespace MyMath; 

float LengthSquared(const Vector3& v1, const Vector3& v2) {
	float dx = v1.x - v2.x;
	float dy = v1.y - v2.y;
	float dz = v1.z - v2.z;
	return dx * dx + dy * dy + dz * dz;
}

StageScene::~StageScene() {
	// Stop all audio to prevent access violation on buffer destruction
	AudioManager::GetInstance()->StopAllWave();
	voiceLockSound_ = nullptr;
	voiceMissileSound_ = nullptr;

	delete player_;
	delete ground_;
	delete debugCamera_;
	delete reticle_;
	delete lockOnMark_;
	delete hpBarSprite_;
	delete lifeIconSprite_;
	delete fadeSprite_;

	delete spriteWave_;
	delete spriteReady_;
	delete spriteStart_;
	delete spriteClear_;
	
	delete minimapBg_;
	delete minimapPlayer_;

	// ポーズメニュースプライト削除
	delete pauseOverlay_;
	delete pauseMenuBg_;
	for (int i = 0; i < 3; ++i) {
		delete pauseMenuItems_[i];
	}

	for (Enemy* e : enemies_)
		delete e;
	for (Explosion* e : explosions_)
		delete e;
}

void StageScene::Initialize() {
	ModelManager* modelManager = ModelManager::GetInstance();
	
	modelManager->LoadModel("player");
	modelManager->LoadModel("enemy");
	modelManager->LoadModel("playerBullet");
	modelManager->LoadModel("enemyBullet");
	modelManager->LoadModel("enemyMissile");
	modelManager->LoadModel("playerMissile");
	
	modelManager->LoadModel("explosion"); 
	
	modelManager->LoadModel("ground");

	playerModel_ = modelManager->FindModel("player");
	enemyModel_ = modelManager->FindModel("enemy");
	playerBulletModel_ = modelManager->FindModel("playerBullet");
	enemyBulletModel_ = modelManager->FindModel("enemyBullet");
	enemyMissileModel_ = modelManager->FindModel("enemyMissile");
	playerMissileModel_ = modelManager->FindModel("playerMissile");
	explosionModel_ = modelManager->FindModel("explosion"); 
	
	groundModel_ = modelManager->FindModel("ground");

	// camera_.Initialize();
	camera_.SetTranslate({ 0.0f, 2.5f, -25.0f });
	camera_.Update();

	modelManager->LoadModel("Skydome");
	skydomeModel_ = modelManager->FindModel("Skydome");
	skydome_ = std::make_unique<Object3d>();
	skydome_->Initialize(Object3dCommon::GetInstance());
	skydome_->SetModel(skydomeModel_);
	skydome_->SetCamera(&camera_);
	skydome_->SetScale({ 1000.0f, 1000.0f, 1000.0f });
	skydome_->Update();

	ParticleManager::GetInstance()->CreateParticleGroup("MissileExhaust", "tex1.png");
	ParticleManager::GetInstance()->SetBlendMode("MissileExhaust", BlendMode::kAdd);
	input_ = Input::GetInstance();
	debugCamera_ = new DebugCamera();

	player_ = new Player();
	player_->Initialize(playerModel_, &camera_);
	player_->SetBulletModel(playerBulletModel_);
	player_->SetMissileModel(playerMissileModel_);

	ground_ = new Ground();
	ground_->Initialize(groundModel_, &camera_);

	reticle_ = new Reticle();
	reticle_->Initialize();

	SpriteCommon* spriteCommon = SpriteCommon::GetInstance();
	TextureManager* texManager = TextureManager::GetInstance();

	TextureManager::GetInstance()->LoadTexture("lockOn.png");
	lockOnTex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("lockOn.png");
	lockOnMark_ = new Sprite();
	lockOnMark_->Initialize(spriteCommon, "lockOn.png");
	lockOnMark_->SetPosition({ 0, 0 });
	lockOnMark_->SetSize({ 64, 64 });
	lockOnMark_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	lockOnMark_->SetAnchorPoint({ 0.5f, 0.5f });

	reticle_ = new Reticle();
	reticle_->Initialize();

	TextureManager::GetInstance()->LoadTexture("white1x1.png");
	uiTexHandle_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("white1x1.png");
	hpBarSprite_ = new Sprite();
	hpBarSprite_->Initialize(spriteCommon, "white1x1.png");
	hpBarSprite_->SetPosition({ 50, 650 });
	hpBarSprite_->SetSize({ 200, 20 });
	hpBarSprite_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	hpBarSprite_->SetAnchorPoint({ 0.0f, 0.0f });

	lifeIconSprite_ = new Sprite();
	lifeIconSprite_->Initialize(spriteCommon, "white1x1.png");
	lifeIconSprite_->SetPosition({ 0, 0 });
	lifeIconSprite_->SetSize({ 20, 20 });
	lifeIconSprite_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f });
	lifeIconSprite_->SetAnchorPoint({ 0.0f, 0.0f });

	fadeTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("white1x1.png");
	fadeSprite_ = new Sprite();
	fadeSprite_->Initialize(spriteCommon, "white1x1.png");
	fadeSprite_->SetPosition({ 0, 0 });
	fadeSprite_->SetSize({ 1280, 720 });
	fadeSprite_->SetColor({ 1, 1, 1, 1 });
	fadeSprite_->SetAnchorPoint({ 0, 0 });
	fadeSprite_->SetTextureLeftTop({ 0.0f, 0.0f });
	fadeSprite_->SetTextureSize({ 1.0f, 1.0f });

	TextureManager::GetInstance()->LoadTexture("text_ready.png");
	TextureManager::GetInstance()->LoadTexture("text_start.png");
	TextureManager::GetInstance()->LoadTexture("text_clear.png");
	texReady_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("text_ready.png");
	texStart_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("text_start.png");
	texClear_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("text_clear.png");

	spriteWave_ = new Sprite();
	spriteWave_->Initialize(spriteCommon, "white1x1.png");
	spriteWave_->SetPosition({ 640, 360 });
	spriteWave_->SetSize({ 300, 60 });
	spriteWave_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f });
	spriteWave_->SetAnchorPoint({ 0.5f, 0.5f });

	spriteReady_ = new Sprite();
	spriteReady_->Initialize(spriteCommon, "text_ready.png");
	spriteReady_->SetPosition({ 640, 360 });
	spriteReady_->SetSize({ 512, 128 });
	spriteReady_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	spriteReady_->SetAnchorPoint({ 0.5f, 0.5f });

	spriteStart_ = new Sprite();
	spriteStart_->Initialize(spriteCommon, "text_start.png");
	spriteStart_->SetPosition({ 640, 360 });
	spriteStart_->SetSize({ 512, 128 });
	spriteStart_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	spriteStart_->SetAnchorPoint({ 0.5f, 0.5f });

	spriteClear_ = new Sprite();
	spriteClear_->Initialize(spriteCommon, "text_clear.png");
	spriteClear_->SetPosition({ 640, 360 });
	spriteClear_->SetSize({ 512, 128 });
	spriteClear_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	spriteClear_->SetAnchorPoint({ 0.5f, 0.5f });

	soundLockOn_ = AudioManager::GetInstance()->SoundLoadFile("./Resources/missile_lock.wav");
	soundMissile_ = AudioManager::GetInstance()->SoundLoadFile("./Resources/missile_search.wav");
	soundExplosion_ = AudioManager::GetInstance()->SoundLoadFile("./Resources/explosion.mp3");

	std::ifstream file("./Resources/enemy_data.json");
	if (file.fail()) {
		// assert(0 && "JSON file not found.");
	} else {
		json deserialized;
		file >> deserialized;

		enemySpawnList_.clear();
		for (const auto& enemyData : deserialized["enemies"]) {
			EnemySpawnData data;
			data.wave = enemyData.value("wave", 1);
			data.spawnTime = enemyData["spawnTime"];
			data.position.x = enemyData["position"][0];
			data.position.y = enemyData["position"][1];
			data.position.z = enemyData["position"][2];
			data.velocity.x = enemyData["velocity"][0];
			data.velocity.y = enemyData["velocity"][1];
			data.velocity.z = enemyData["velocity"][2];
			data.type = enemyData.value("type", "A");
			data.attackPattern = enemyData.value("attackPattern", "Normal");
			enemySpawnList_.push_back(data);
		}
		enemySpawnList_.sort([](const EnemySpawnData& a, const EnemySpawnData& b) {
			if (a.wave != b.wave)
				return a.wave < b.wave;
			return a.spawnTime < b.spawnTime;
			});
	}

	score_ = 0;
	gameLimitTimer_ = 60 * 180;

	currentWave_ = 1;
	waveState_ = WaveState::Intro;
	waveTimer_ = 0;

	// ポーズメニュー初期化
	isPaused_ = false;
	pauseMenuSelection_ = PauseMenuItem::Resume;
	requestRestart_ = false;

	// ポーズオーバーレイ（半透明の暗い背景）
	pauseOverlay_ = new Sprite();
	pauseOverlay_->Initialize(spriteCommon, "white1x1.png");
	pauseOverlay_->SetPosition({ 0, 0 });
	pauseOverlay_->SetSize({ 1280, 720 });
	pauseOverlay_->SetColor({ 0.0f, 0.0f, 0.0f, 0.5f });
	pauseOverlay_->SetAnchorPoint({ 0.0f, 0.0f });

	// ポーズメニュー背景
	pauseMenuBg_ = new Sprite();
	pauseMenuBg_->Initialize(spriteCommon, "white1x1.png");
	pauseMenuBg_->SetPosition({ 640, 360 });
	pauseMenuBg_->SetSize({ 400, 300 });
	pauseMenuBg_->SetColor({ 0.1f, 0.1f, 0.2f, 0.9f });
	pauseMenuBg_->SetAnchorPoint({ 0.5f, 0.5f });

	// メニュー項目（Resume, Restart, Back to Title）
	const char* menuLabels[3] = { "RESUME", "RESTART", "TITLE" };
	for (int i = 0; i < 3; ++i) {
		pauseMenuItems_[i] = new Sprite();
		pauseMenuItems_[i]->Initialize(spriteCommon, "white1x1.png");
		pauseMenuItems_[i]->SetPosition({ 640.0f, 280.0f + i * 70.0f });
		pauseMenuItems_[i]->SetSize({ 300, 50 });
		pauseMenuItems_[i]->SetColor({ 0.3f, 0.3f, 0.4f, 1.0f });
		pauseMenuItems_[i]->SetAnchorPoint({ 0.5f, 0.5f });
	}

	// Minimap Sprites
	minimapBg_ = new Sprite();
	minimapBg_->Initialize(spriteCommon, "white1x1.png");
	minimapBg_->SetColor({ 0.0f, 0.0f, 0.0f, 0.5f }); // Semi-transparent black
	minimapBg_->SetAnchorPoint({ 0.0f, 0.0f });

	minimapPlayer_ = new Sprite();
	minimapPlayer_->Initialize(spriteCommon, "white1x1.png");
	minimapPlayer_->SetSize({ 8.0f, 8.0f });
	minimapPlayer_->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f }); // Cyan
	minimapPlayer_->SetAnchorPoint({ 0.5f, 0.5f });

	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;
}

std::optional<SceneID> StageScene::Update() {
	// ポーズ中はポーズ更新のみ
	if (isPaused_) {
		return UpdatePause();
	}

	if (phase_ == ScenePhase::kFadeIn)
		return UpdateFadeIn();
	if (phase_ == ScenePhase::kMain)
		return UpdateMain();
	if (phase_ == ScenePhase::kFadeOut)
		return UpdateFadeOut();
	return std::nullopt;
}

std::optional<SceneID> StageScene::UpdateFadeIn() {
	if (--fadeTimer_ <= 0)
		phase_ = ScenePhase::kMain;
	return std::nullopt;
}

void StageScene::Finalize() {
}

std::optional<SceneID> StageScene::UpdateFadeOut() {
	if (++fadeTimer_ >= kFadeDuration_)
		return SceneID::kClear;
	return std::nullopt;
}

std::optional<SceneID> StageScene::UpdateMain() {
	// ESCキーでポーズ
	if (input_->TriggerKey(DIK_ESCAPE)) {
		isPaused_ = true;
		pauseMenuSelection_ = PauseMenuItem::Resume;
		return std::nullopt;
	}

	// リスタートリクエストがある場合、シーンを再初期化
	if (requestRestart_) {
		requestRestart_ = false;
		// 現在のリソースをクリーンアップして再初期化
		// 簡易的にタイトルに戻ってからステージに遷移
		return SceneID::kStage;
	}

	gameLimitTimer_--;
	ground_->Update(); 
	if(skydome_) skydome_->Update(); 
	if(skydome_) skydome_->Update(); 
	ParticleManager::GetInstance()->Update(); 
	
	// PlayerMissile::EditOffset(); 

	// ==========================================
	// ==========================================
	bool isGameOver = false;

	if (player_->IsDead()) {
		isGameOver = true;
	} else if (gameLimitTimer_ <= 0) {
		isGameOver = true;
	}

	if (isGameOver) {
		ClearScene::isWin = false;
		ClearScene::finalScore = score_;
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0;
		return std::nullopt; 
	}

	// ==========================================
	// ==========================================
	switch (waveState_) {
	case WaveState::Intro:
		waveTimer_++;
		if (waveTimer_ > 180) { 
			waveState_ = WaveState::Battle;
			waveTimer_ = 0;
		}
		break;

	case WaveState::Battle:
		waveTimer_++; 

		{
			auto it = enemySpawnList_.begin();
			while (it != enemySpawnList_.end()) {
				if (it->wave != currentWave_)
					break;
				if (it->spawnTime > waveTimer_)
					break;

				Enemy* newEnemy = new Enemy();
				newEnemy->SetBulletModel(enemyBulletModel_);
				newEnemy->SetMissileModel(enemyMissileModel_);
				newEnemy->SetPlayer(player_);
				newEnemy->Initialize(enemyModel_, it->position, it->velocity, it->type, it->attackPattern, &camera_);
				enemies_.push_back(newEnemy);

				it = enemySpawnList_.erase(it);
			}
		}

		{
			bool isSpawnFinished = true;
			for (const auto& d : enemySpawnList_) {
				if (d.wave == currentWave_) {
					isSpawnFinished = false;
					break;
				}
			}

			if (enemies_.empty() && isSpawnFinished) {
				waveState_ = WaveState::Clear;
				waveTimer_ = 0;
			}
		}
		break;

	case WaveState::Clear:
		waveTimer_++;
		if (waveTimer_ > 120) { 
			bool hasNextWave = false;
			for (const auto& d : enemySpawnList_) {
				if (d.wave > currentWave_) {
					hasNextWave = true;
					break;
				}
			}

			if (hasNextWave) {
				currentWave_++;
				waveState_ = WaveState::Intro;
				waveTimer_ = 0;
			} else {
				ClearScene::isWin = true;
				ClearScene::finalScore = score_;
				phase_ = ScenePhase::kFadeOut;
				fadeTimer_ = 0;
			}
		}
		break;
	}

	// ==========================================
	// ==========================================

	bool isPlayerActive = (waveState_ == WaveState::Battle) && !player_->IsDead();

	player_->Update(isPlayerActive);

	enemies_.remove_if([this](Enemy* e) {
		if (e->IsDead()) {
			// 敵を削除する前に、この敵をターゲットにしているミサイルのターゲットをクリア
			player_->ClearMissileTargetsFor(e);
			// ロックオン中の敵が削除される場合、ロックオンを解除
			if (lockedEnemy_ == e) {
				lockedEnemy_ = nullptr;
			}
			delete e;
			return true;
		}
		return false;
		});
	for (Enemy* e : enemies_)
		e->Update();

	explosions_.remove_if([](Explosion* e) {
		if (e->IsDead()) {
			delete e;
			return true;
		}
		return false;
		});
	for (Explosion* e : explosions_)
		e->Update();

	const float kPlayerRadius = 1.0f;
	const float kEnemyRadius = 4.0f;
	const float kBulletRadius = 0.5f;

	for (PlayerBullet* pBullet : player_->GetBullets()) {
		for (Enemy* enemy : enemies_) {
			if (pBullet->IsDead() || enemy->IsDead())
				continue;
			if (LengthSquared(pBullet->GetWorldPosition(), enemy->GetWorldPosition()) < pow(kBulletRadius + kEnemyRadius, 2)) {
				pBullet->OnCollision();
				enemy->OnCollision(pBullet->GetPower());
				if (enemy->IsDead()) {
					score_ += 100;
					AudioManager::GetInstance()->SoundPlayWave(soundExplosion_);
					camera_.Shake(0.2f, 10);  // 敵撃破時に軽いシェイク
					Explosion* newExp = new Explosion();
					newExp->Initialize(explosionModel_, enemy->GetWorldPosition(), &camera_);
					explosions_.push_back(newExp);
				}
			}
		}
	}

	for (Enemy* enemy : enemies_) {
		for (EnemyBullet* eBullet : enemy->GetBullets()) {
			if (eBullet->IsDead())
				continue;
			if (LengthSquared(eBullet->GetWorldPosition(), player_->GetWorldPosition()) < pow(kBulletRadius + kPlayerRadius, 2)) {
				eBullet->OnCollision();
				player_->OnCollision();
			}
		}
		for (EnemyMissile* eMissile : enemy->GetMissiles()) {
			if (eMissile->IsDead())
				continue;
			if (LengthSquared(eMissile->GetWorldPosition(), player_->GetWorldPosition()) < pow(kBulletRadius + kPlayerRadius, 2)) {
				eMissile->OnCollision();
				player_->OnCollision();


				Explosion* newExp = new Explosion();
				newExp->Initialize(explosionModel_, eMissile->GetWorldPosition(), &camera_);
				explosions_.push_back(newExp);
			}
		}

	}

	for (PlayerMissile* missile : player_->GetMissiles()) {
		for (Enemy* enemy : enemies_) {
			if (missile->IsDead() || enemy->IsDead())
				continue;
			if (LengthSquared(missile->GetWorldPosition(), enemy->GetWorldPosition()) < pow(kBulletRadius + kEnemyRadius, 2)) {
				missile->OnCollision();
				enemy->OnCollision(missile->GetPower());
				if (enemy->IsDead()) {
					score_ += 500;
					AudioManager::GetInstance()->SoundPlayWave(soundExplosion_);
					camera_.Shake(0.4f, 15);  // ミサイル撃破は強め
					Explosion* newExp = new Explosion();
					newExp->Initialize(explosionModel_, enemy->GetWorldPosition(), &camera_);
					explosions_.push_back(newExp);
				}
			}
		}
	}

	for (Enemy* enemy : enemies_) {
		if (enemy->IsDead())
			continue;
		if (LengthSquared(enemy->GetWorldPosition(), player_->GetWorldPosition()) < pow(kEnemyRadius + kPlayerRadius, 2)) {
			enemy->OnCollision(100);
			player_->OnCollision();

			if (enemy->IsDead()) {
				AudioManager::GetInstance()->SoundPlayWave(soundExplosion_);
				camera_.Shake(0.5f, 20);  // 衝突は最も強いシェイク
				Explosion* newExp = new Explosion();
				newExp->Initialize(explosionModel_, enemy->GetWorldPosition(), &camera_);
				explosions_.push_back(newExp);
			}
		}
	}

	if (isDebugCameraActive_) {
		// debugCamera_->Update();
		// camera_.SetRotate(debugCamera_->GetCamera().GetRotate());
		// camera_.SetTranslate(debugCamera_->GetCamera().GetTranslate());
		// camera_.Update();
	} else {
		Vector3 pPos = player_->GetWorldPosition();
		Vector3 pRot = player_->GetRotation();
		Vector3 tCamPos = { pPos.x, pPos.y + 4.0f, pPos.z - 30.0f };
		Vector3 camPos = camera_.GetTranslate();
		camPos.x = LerpShort(camPos.x, tCamPos.x, 0.1f);
		camPos.y = LerpShort(camPos.y, tCamPos.y, 0.1f);
		camPos.z = LerpShort(camPos.z, tCamPos.z, 0.1f);
		camera_.SetTranslate(camPos);

		float tRoll = -pRot.z * 1.0f;
		float tPitch = -pRot.x * 0.5f;
		Vector3 camRot = camera_.GetRotate();
		camRot.z = LerpShort(camRot.z, tRoll, 0.1f);
		camRot.x = LerpShort(camRot.x, tPitch, 0.1f);
		camera_.SetRotate(camRot);


		camera_.Update();
	}

	Vector3 pPos = player_->GetWorldPosition();
	Vector3 pRot = player_->GetRotation();
	Matrix4x4 matPlayer = MakeAffineMatrix({ 1, 1, 1 }, pRot, pPos);
	Vector3 targetPos = TransformCoord({ 0, 0, -50.0f }, matPlayer);
	reticle_->Update(targetPos, camera_);

	lockedEnemy_ = nullptr;
	float minDst = 100.0f;
	Vector2 rPos = reticle_->GetPosition();
	for (Enemy* e : enemies_) {
		if (e->IsDead())
			continue;
		Vector3 ePos = e->GetWorldPosition();
		if (ePos.z < camera_.GetTranslate().z)
			continue;
		Vector2 eScr = WorldToScreen(ePos, camera_.GetViewMatrix(), camera_.GetProjectionMatrix(), 1280, 720);
		float dst = sqrtf(powf(eScr.x - rPos.x, 2) + powf(eScr.y - rPos.y, 2));
		if (dst < minDst) {
			minDst = dst;
			lockedEnemy_ = e;
		}
	}

	// Keep track of previously locked enemy to detect changes
	static Enemy* prevLockedEnemy = nullptr;

	if (lockedEnemy_) {
		// New Lock or Changed Target
		if (lockedEnemy_ != prevLockedEnemy) {
			// Stop previous sound if any
			if (voiceLockSound_) {
				AudioManager::GetInstance()->StopWave(voiceLockSound_);
				voiceLockSound_ = nullptr;
			}

			// Play new sound
			voiceLockSound_ = AudioManager::GetInstance()->SoundPlayWave(soundLockOn_);
			isLockSoundPlayed_ = true;
		}
	} else {
		// No lock, stop sound if playing
		if (voiceLockSound_) {
			AudioManager::GetInstance()->StopWave(voiceLockSound_);
			voiceLockSound_ = nullptr;
		}
		isLockSoundPlayed_ = false;
	}
	prevLockedEnemy = lockedEnemy_;

	// Check tracking missile status
	if (voiceMissileSound_ && trackingMissile_) {
		bool isAlive = false;
		for (const auto* m : player_->GetMissiles()) {
			if (m == trackingMissile_) {
				isAlive = true;
				break;
			}
		}
		if (!isAlive) {
			AudioManager::GetInstance()->StopWave(voiceMissileSound_);
			voiceMissileSound_ = nullptr;
			trackingMissile_ = nullptr;
		}
	}

	if (isPlayerActive) {
		if (input_->PushMouse(1)) {
			if (lockedEnemy_) {
				PlayerMissile* m = player_->FireMissile(lockedEnemy_);
				if (m) {
					if (voiceMissileSound_) {
						AudioManager::GetInstance()->StopWave(voiceMissileSound_);
						voiceMissileSound_ = nullptr;
					}
					voiceMissileSound_ = AudioManager::GetInstance()->SoundPlayWave(soundMissile_);
					trackingMissile_ = m;
				}
			}
		}
	}

	float hpRatio = (float)player_->GetHP() / (float)player_->GetMaxHP();
	if (hpRatio < 0.0f)
		hpRatio = 0.0f;
	hpBarSprite_->SetSize({ 200.0f * hpRatio, 20.0f });

	return std::nullopt;
}

std::optional<SceneID> StageScene::UpdatePause() {
	// ESCキーまたはEnterでResume選択時にポーズ解除
	if (input_->TriggerKey(DIK_ESCAPE)) {
		isPaused_ = false;
		return std::nullopt;
	}

	// 上下キーでメニュー選択
	if (input_->TriggerKey(DIK_UP) || input_->TriggerKey(DIK_W)) {
		int sel = static_cast<int>(pauseMenuSelection_);
		sel--;
		if (sel < 0) sel = static_cast<int>(PauseMenuItem::Count) - 1;
		pauseMenuSelection_ = static_cast<PauseMenuItem>(sel);
	}
	if (input_->TriggerKey(DIK_DOWN) || input_->TriggerKey(DIK_S)) {
		int sel = static_cast<int>(pauseMenuSelection_);
		sel++;
		if (sel >= static_cast<int>(PauseMenuItem::Count)) sel = 0;
		pauseMenuSelection_ = static_cast<PauseMenuItem>(sel);
	}

	// Enterキーまたはスペースで決定
	if (input_->TriggerKey(DIK_RETURN) || input_->TriggerKey(DIK_SPACE)) {
		switch (pauseMenuSelection_) {
		case PauseMenuItem::Resume:
			isPaused_ = false;
			break;
		case PauseMenuItem::Restart:
			isPaused_ = false;
			requestRestart_ = true;
			break;
		case PauseMenuItem::BackToTitle:
			isPaused_ = false;
			return SceneID::kTitle;
		default:
			break;
		}
	}

	// メニュー項目の色更新（選択中は明るく）
	for (int i = 0; i < 3; ++i) {
		if (i == static_cast<int>(pauseMenuSelection_)) {
			pauseMenuItems_[i]->SetColor({ 0.8f, 0.8f, 1.0f, 1.0f });
		} else {
			pauseMenuItems_[i]->SetColor({ 0.3f, 0.3f, 0.4f, 1.0f });
		}
	}

	return std::nullopt;
}

void StageScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	if (phase_ != ScenePhase::kFadeIn) {
		Object3dCommon::GetInstance()->SetupCommonState();
		if (skydome_) skydome_->Draw();
		if(ground_) ground_->Draw(); 
		player_->Draw();
		for (Enemy* e : enemies_)
			if (!e->IsDead())
				e->Draw();
		for (Explosion* e : explosions_)
			e->Draw();
		ParticleManager::GetInstance()->Draw();
	}

	SpriteCommon::GetInstance()->SetupCommonState();
	if (lockedEnemy_ && !lockedEnemy_->IsDead()) {
		Vector3 ePos = lockedEnemy_->GetWorldPosition();
		Vector2 sPos = WorldToScreen(ePos, camera_.GetViewMatrix(), camera_.GetProjectionMatrix(), 1280, 720);
		lockOnMark_->SetPosition(sPos);
		lockOnMark_->Update();
		lockOnMark_->Draw();
	}
	if (reticle_)
		reticle_->Draw();

	if (hpBarSprite_) {
		hpBarSprite_->Update();
		hpBarSprite_->Draw();
	}
	// Minimap Drawing
	// Map Settings
	Vector2 mapPos = { 1050.0f, 500.0f }; // Bottom Right
	Vector2 mapSize = { 200.0f, 200.0f };
	float mapScale = 1.5f; // 1m = 1.5px

	// Draw BG
	minimapBg_->SetPosition(mapPos);
	minimapBg_->SetSize(mapSize);
	minimapBg_->Update();
	minimapBg_->Draw();

	// Draw Enemies
	Vector2 mapCenter = { mapPos.x + mapSize.x * 0.5f, mapPos.y + mapSize.y * 0.5f };
	float halfW = mapSize.x * 0.5f - 4.0f; // Margin
	float halfH = mapSize.y * 0.5f - 4.0f;

	for (Enemy* e : enemies_) {
		if (e->IsDead()) continue;
		Vector3 ePos = e->GetWorldPosition();
		// Project to map
		float mx = ePos.x * mapScale;
		float my = -ePos.z * mapScale;

		// Clamp to bounds (Radar style)
		mx = std::clamp(mx, -halfW, halfW);
		my = std::clamp(my, -halfH, halfH);

		e->DrawMinimap({ mapCenter.x + mx, mapCenter.y + my });
	}

	// Draw Player
	Vector3 pPos = player_->GetWorldPosition();
	float px = pPos.x * mapScale;
	float py = -pPos.z * mapScale;
	
	// Clamp player too just in case
	px = std::clamp(px, -halfW, halfW);
	py = std::clamp(py, -halfH, halfH);

	minimapPlayer_->SetPosition({ mapCenter.x + px, mapCenter.y + py });
	minimapPlayer_->Update();
	minimapPlayer_->Update();
	minimapPlayer_->Draw();


	if (waveState_ == WaveState::Intro) {
		if (waveTimer_ < 60) {
			// (Wave display logic omitted for brevity, keeping existing structure)
			// Adjust logic to show READY -> START
		} else if (waveTimer_ < 120) {
			// READY...
			if (spriteReady_) {
				spriteReady_->Update();
				spriteReady_->Draw();
			}
		} else {
			// START!!!
			if (spriteStart_) {
				spriteStart_->Update();
				spriteStart_->Draw();
			}
		}
	} else if (waveState_ == WaveState::Clear) {
		// CLEAR!!
		if (spriteClear_) {
			spriteClear_->Update();
			spriteClear_->Draw();
		}
	}

	// ポーズメニュー描画
	if (isPaused_) {
		if (pauseOverlay_) {
			pauseOverlay_->Update();
			pauseOverlay_->Draw();
		}
		if (pauseMenuBg_) {
			pauseMenuBg_->Update();
			pauseMenuBg_->Draw();
		}
		for (int i = 0; i < 3; ++i) {
			if (pauseMenuItems_[i]) {
				pauseMenuItems_[i]->Update();
				pauseMenuItems_[i]->Draw();
			}
		}
	}

#ifdef USE_IMGUI
	// Debug HUD
	ImGui::Begin("HUD");
	ImGui::Text("WAVE: %d", currentWave_);
	ImGui::Text("SCORE: %06d", score_);
	ImGui::Text("HP: %d / %d", player_->GetHP(), player_->GetMaxHP());
	ImGui::Text("LIVES: %d", player_->GetLives());
	ImGui::End();
#endif

	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn)
		alpha = (float)fadeTimer_ / kFadeDuration_;
	else if (phase_ == ScenePhase::kFadeOut)
		alpha = (float)fadeTimer_ / kFadeDuration_;
	if (alpha > 0.0f && fadeSprite_) {
		fadeSprite_->SetColor({ 1, 1, 1, alpha });
		fadeSprite_->Update();
		fadeSprite_->Draw();
	}
}
