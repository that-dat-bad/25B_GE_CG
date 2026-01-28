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
#include "Input.h"
#include "DebugCamera.h"
#include "Camera.h"
#include "Object3d.h"
#include "Math/MyMath.h"

using namespace MyMath;

#include <assert.h>
#include <cmath>
#include <fstream>

using json = nlohmann::json;
using namespace MyMath; 

float LengthSquared(const Vector3& v1, const Vector3& v2) {
	float dx = v1.x - v2.x;
	float dy = v1.y - v2.y;
	float dz = v1.z - v2.z;
	return dx * dx + dy * dy + dz * dz;
}

StageScene::~StageScene() {

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
	

	modelManager->LoadModel("ground");

	playerModel_ = modelManager->FindModel("player");
	enemyModel_ = modelManager->FindModel("enemy");
	playerBulletModel_ = modelManager->FindModel("playerBullet");
	enemyBulletModel_ = modelManager->FindModel("enemyBullet");
	enemyMissileModel_ = modelManager->FindModel("enemyMissile");
	playerMissileModel_ = modelManager->FindModel("playerMissile");
	// explosionModel_ = modelManager->FindModel("explosion"); 
	
	groundModel_ = modelManager->FindModel("ground");

	// camera_.Initialize();
	camera_.SetTranslate({ 0.0f, 2.5f, -15.0f });
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

	TextureManager::GetInstance()->LoadTexture("white1x1.png");
	texWave_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("white1x1.png");
	texReady_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("white1x1.png");
	texStart_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("white1x1.png");
	texClear_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("white1x1.png");

	spriteWave_ = new Sprite();
	spriteWave_->Initialize(spriteCommon, "white1x1.png");
	spriteWave_->SetPosition({ 640, 360 });
	spriteWave_->SetSize({ 300, 60 });
	spriteWave_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f });
	spriteWave_->SetAnchorPoint({ 0.5f, 0.5f });

	spriteReady_ = new Sprite();
	spriteReady_->Initialize(spriteCommon, "white1x1.png");
	spriteReady_->SetPosition({ 640, 360 });
	spriteReady_->SetSize({ 300, 60 });
	spriteReady_->SetColor({ 1.0f, 0.5f, 0.0f, 1.0f });
	spriteReady_->SetAnchorPoint({ 0.5f, 0.5f });

	spriteStart_ = new Sprite();
	spriteStart_->Initialize(spriteCommon, "white1x1.png");
	spriteStart_->SetPosition({ 640, 360 });
	spriteStart_->SetSize({ 400, 80 });
	spriteStart_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	spriteStart_->SetAnchorPoint({ 0.5f, 0.5f });

	spriteClear_ = new Sprite();
	spriteClear_->Initialize(spriteCommon, "white1x1.png");
	spriteClear_->SetPosition({ 640, 360 });
	spriteClear_->SetSize({ 400, 80 });
	spriteClear_->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f });
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

	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;
}

std::optional<SceneID> StageScene::Update() {
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
// ... same logic ...
	gameLimitTimer_--;
	ground_->Update(); 

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

	enemies_.remove_if([](Enemy* e) {
		if (e->IsDead()) {
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
		Vector3 tCamPos = { pPos.x, pPos.y + 4.0f, pPos.z - 20.0f };
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

	if (lockedEnemy_) {
		if (!isLockSoundPlayed_) {
			AudioManager::GetInstance()->SoundPlayWave(soundLockOn_);
			isLockSoundPlayed_ = true;
		}
	} else {
		isLockSoundPlayed_ = false;
	}

	if (isPlayerActive) {
		if (input_->PushMouse(1)) {
			if (lockedEnemy_) {
				player_->FireMissile(lockedEnemy_);
				AudioManager::GetInstance()->SoundPlayWave(soundMissile_);
			}
		}
	}

	float hpRatio = (float)player_->GetHP() / (float)player_->GetMaxHP();
	if (hpRatio < 0.0f)
		hpRatio = 0.0f;
	hpBarSprite_->SetSize({ 200.0f * hpRatio, 20.0f });

	return std::nullopt;
}

void StageScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	if (phase_ != ScenePhase::kFadeIn) {
		Object3dCommon::GetInstance()->SetupCommonState();
		if(ground_) ground_->Draw(); 
		player_->Draw();
		for (Enemy* e : enemies_)
			if (!e->IsDead())
				e->Draw();
		for (Explosion* e : explosions_)
			e->Draw();
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
	int lives = player_->GetLives();
	for (int i = 0; i < lives; i++) {
		lifeIconSprite_->SetPosition({ 50.0f + i * 25.0f, 620.0f });
		lifeIconSprite_->Update();
		lifeIconSprite_->Draw();
	}

	if (waveState_ == WaveState::Intro) {
		if (waveTimer_ < 60) {
			// WAVE
			if (spriteWave_) {
				spriteWave_->Update();
				spriteWave_->Draw();
			}
		} else if (waveTimer_ < 120) {
			// WAVE... READY...
			if (spriteWave_) {
				spriteWave_->Update();
				spriteWave_->Draw();
			}
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
