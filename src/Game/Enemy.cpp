#include "Enemy.h"
#include "ChainBomb.h"
#include "Player.h"
#include "ModelManager.h"
#include "CameraManager.h"
#include "TextureManager.h"
#include "TDEngine.h"

// 攻撃クラスのヘッダー
#include "Beam.h"
#include "Needle.h"
#include "Thunder.h"
#include "Punch.h"
#include "DeathEx.h"
#include "EnemyDeathParticle.h"

using namespace MyMath;

// ---------------------------------------------------------
// デストラクタ
// ---------------------------------------------------------
Enemy::~Enemy() {
	if (object3d_) delete object3d_;
	if (rand_) delete rand_;
	if (beam_) delete beam_;

	for (auto p : needles_) delete p;
	for (auto p : thunders_) delete p;
	for (auto p : punches_) delete p;
	for (auto p : deathExs_) delete p;
	for (auto p : deathParticles_) delete p;

	AudioManager* audio = TDEngine::GetAudioManager();
	//audio->StopAllVoices();
	pBgmVoice_ = nullptr;
	audio->SoundUnload(&boundSe_);
	audio->SoundUnload(&approachSe_);
	audio->SoundUnload(&beamSe_);
	audio->SoundUnload(&needleSe_);
	audio->SoundUnload(&thunderSe_);
	audio->SoundUnload(&deathSe_);
	audio->SoundUnload(&changeSe_);
}

// ---------------------------------------------------------
// 初期化
// ---------------------------------------------------------
void Enemy::Initialize(const Vector3& position) {
	// モデル読み込み (Resources/enemy.obj 前提)
	std::string modelPath = "Resources/enemy/enemy.obj";
	ModelManager::GetInstance()->LoadModel(modelPath);

	// Object3d生成
	object3d_ = Object3d::Create();
	object3d_->SetModel(modelPath);
	object3d_->SetTranslate(position);
	object3d_->SetScale(originalScale_);

	// 色初期化
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	object3d_->SetColor(color_);

	// ランダム生成器
	rand_ = new Rand();
	rand_->Initialize();
	rand_->RandomInitialize();
	randomValue = static_cast<int>(rand_->GetRandom());

	// 初期行動
	behaviorRequest_ = Behavior::kStart;

	// SE
	AudioManager* audio = TDEngine::GetAudioManager();
	boundSe_ = audio->SoundLoadWave("Resources/Sound/bound.wav");
	approachSe_ = audio->SoundLoadWave("Resources/Sound/approach.wav");
	beamSe_ = audio->SoundLoadWave("Resources/Sound/beam.wav");
	needleSe_ = audio->SoundLoadWave("Resources/Sound/needle.wav");
	thunderSe_ = audio->SoundLoadWave("Resources/Sound/thunder.wav");
	deathSe_ = audio->SoundLoadWave("Resources/Sound/enemyDeath.wav");
}

// ---------------------------------------------------------
// 更新
// ---------------------------------------------------------
void Enemy::Update() {

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		if (turnTimer_ <= 1.0f) {
			turnTimer_ += 0.01f;
		}

		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> *3.0f / 2.0f, // kLeft
			std::numbers::pi_v<float> / 2.0f,        // kRight
			std::numbers::pi_v<float>                // kFront
		};

		// 状態に応じた角度を取得
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(direction_)];

		// 現在の回転を取得して補間
		Vector3 rot = object3d_->GetRotate();
		// worldTransform_.EaseInFloat -> MyMath::EaseIn
		// EaseIn(t, start, end) を使用
		rot.y = EaseIn(turnTimer_, turnFirstRotationY, destinationRotationY);
		object3d_->SetRotate(rot);
	}

	if (isHit_) {
		HitTimer();
	}

	// 振る舞い変更リクエスト処理
	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;
		// 各振る舞いの初期化
		switch (behavior_) {
		case Behavior::kRoot:     BehaviorRootInitialize(); break;
		case Behavior::kBound:    BehaviorBoundInitialize(); break;
		case Behavior::kRound:    BehaviorRoundInitialize(); break;
		case Behavior::kBeam:     BehaviorBeamInitialize(); break;
		case Behavior::kApproach: BehaviorApproachInitialize(); break;
		case Behavior::kNeedle:   BehaviorNeedleInitialize(); break;
		case Behavior::kThunder:  BehaviorThunderInitialize(); break;
		case Behavior::kPunch:    BehaviorPunchInitialize(); break;
		case Behavior::kDeath:    BehaviorDeathInitialize(); break;
		case Behavior::kChange:   BehaviorChangeInitialize(); break;
		case Behavior::kStart:    BehaviorStartInitialize(); break;
		}
		behaviorRequest_ = Behavior::kUnknown;
	}

	// 各振る舞いの更新
	switch (behavior_) {
	case Behavior::kRoot:     BehaviorRootUpdate(); break;
	case Behavior::kBound:    BehaviorBoundUpdate(); break;
	case Behavior::kRound:    BehaviorRoundUpdate(); break;
	case Behavior::kBeam:     BehaviorBeamUpdate(); break;
	case Behavior::kApproach: BehaviorApproachUpdate(); break;
	case Behavior::kNeedle:   BehaviorNeedleUpdate(); break;
	case Behavior::kThunder:  BehaviorThunderUpdate(); break;
	case Behavior::kPunch:    BehaviorPunchUpdate(); break;
	case Behavior::kDeath:    BehaviorDeathUpdate(); break;
	case Behavior::kChange:   BehaviorChangeUpdate(); break;
	case Behavior::kStart:    BehaviorStartUpdate(); break;
	}

	// Object3d更新
	if (object3d_) object3d_->Update();
}

// ---------------------------------------------------------
// 描画
// ---------------------------------------------------------
void Enemy::Draw() {
	if (isDead_) return;

	// ヒット時の点滅処理
	if (isHit_) {
		if (hitTimer_ % 2 == 0) {
			if (object3d_) object3d_->Draw();
		}
	} else {
		if (object3d_) object3d_->Draw();
	}

	// 各攻撃オブジェクト描画
	if (beam_) beam_->Draw();
	for (auto p : needles_) if (p) p->Draw();
	for (auto p : thunders_) if (p) p->Draw();
	for (auto p : punches_) if (p) p->Draw();

	// エフェクト描画
	for (auto p : deathExs_) if (p) p->Draw();
	for (auto p : deathParticles_) if (p) p->Draw();
}

// ---------------------------------------------------------
// Behavior: Bound (跳ね回る)
// ---------------------------------------------------------
void Enemy::BehaviorBoundInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 50.0f;
	attackLingeringTimer_ = 40.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 36.0f, -20.0f, 0.0f };
	attackCount_ = 0;
	enemySpeedDecay_ = { 0.0f, 0.1f, 0.0f };
	t = 0.0f;
}

void Enemy::BehaviorBoundUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			enemySpeed_ = { -0.5f, 2.0f, 0.0f };
		}
		break;

	case AttackPhase::kAttack:
		if (attackCount_ >= 3) {
			if (attackParameter_ >= attackRushTimer_) {
				attackPhase_ = AttackPhase::kLingering;
				attackParameter_ = 0;
				t = 0.0f;
			}
		} else {
			pos = Add(pos, enemySpeed_);
			object3d_->SetTranslate(pos);

			if (enemySpeed_.y <= -2.0f) {
				if (attackAfterTimer_ >= 0.0f) {
					attackAfterTimer_--;
					pos.y = -20.0f;
					object3d_->SetTranslate(pos);
				} else {
					Vector3 playerPos = player_->GetWorldPosition();
					Vector3 myPos = object3d_->GetTranslate();
					Vector3 myRot = object3d_->GetRotate();

					if (playerPos.x >= myPos.x) {
						enemySpeed_.x = 0.5f;
						direction_ = Direction::kRight;
						turnFirstRotationY = myRot.y;
						turnTimer_ = kTimeTurn;
					} else {
						enemySpeed_.x = -0.5f;
						direction_ = Direction::kLeft;
						turnFirstRotationY = myRot.y;
						turnTimer_ = kTimeTurn;
					}
					enemySpeed_.y = 2.0f;
					attackCount_++;
					attackAfterTimer_ = 10.0f;
					TDEngine::GetAudioManager()->SoundPlayWave(boundSe_, false, 1.0f);
				}
			} else {
				enemySpeed_.y -= enemySpeedDecay_.y;
			}
		}
		break;

	case AttackPhase::kLingering:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;

			Vector3 rot = object3d_->GetRotate();
			turnFirstRotationY = rot.y;
			turnTimer_ = kTimeTurn;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Round (往復)
// ---------------------------------------------------------
void Enemy::BehaviorRoundInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 100.0f;
	attackLingeringTimer_ = 40.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 36.0f, 20.0f, 0.0f };
	attackCount_ = 0;
	enemySpeedDecay_ = { 0.0f, 0.1f, 0.0f };
	t = 0.0f;
}

void Enemy::BehaviorRoundUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			enemySpeed_ = { -1.8f, -2.0f, 0.0f };
		}
		break;

	case AttackPhase::kAttack:
		if (attackCount_ >= 2) {
			if (attackParameter_ >= attackRushTimer_) {
				attackPhase_ = AttackPhase::kLingering;
				attackParameter_ = 0;
				t = 0.0f;
			}
		} else {
			if (enemySpeed_.y >= 1.9f) {
				if (attackAfterTimer_ >= 0.0f) {
					attackAfterTimer_--;
					break;
				}
				enemySpeed_.x *= -1.0f;
				enemySpeed_.y = -2.0f;
				attackCount_++;
				attackAfterTimer_ = 10.0f;

				direction_ = Direction::kRight;
				Vector3 rot = object3d_->GetRotate();
				turnFirstRotationY = rot.y;
				turnTimer_ = kTimeTurn;
			} else {
				enemySpeed_.y += enemySpeedDecay_.y;
			}
			pos = Add(pos, enemySpeed_);
			object3d_->SetTranslate(pos);
		}
		break;

	case AttackPhase::kLingering:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			Vector3 rot = object3d_->GetRotate();
			turnFirstRotationY = rot.y;
			turnTimer_ = kTimeTurn;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Beam (ビーム)
// ---------------------------------------------------------
void Enemy::BehaviorBeamInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 50.0f;
	attackLingeringTimer_ = 40.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 36.0f, -5.0f, 0.0f };
	t = 0.0f;
}

void Enemy::BehaviorBeamUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;

			// Beam生成
			beam_ = new Beam();
			beam_->Initialize(pos);
			TDEngine::GetAudioManager()->SoundPlayWave(beamSe_, false, 1.0f);
		}
		break;

	case AttackPhase::kAttack:
		if (beam_) beam_->Update();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			delete beam_;
			beam_ = nullptr;
		}
		break;

	case AttackPhase::kLingering:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Approach (接近)
// ---------------------------------------------------------
void Enemy::BehaviorApproachInitialize() {
	attackParameter_ = 0;
	enemySpeed_ = { 0.0f, 0.0f, 1.0f };
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 50.0f;
	attackLingeringTimer_ = 40.0f;

	Vector3 pPos = player_->GetWorldPosition();
	initPos_ = { pPos.x, pPos.y, 20.0f };
	t = 0.0f;
}

void Enemy::BehaviorApproachUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			enemySpeed_ = { 0.0f, 0.0f, -2.0f };
			attackParameter_ = 0;

			direction_ = Direction::kFront;
			Vector3 rot = object3d_->GetRotate();
			turnFirstRotationY = rot.y;
			turnTimer_ = kTimeTurn;
			TDEngine::GetAudioManager()->SoundPlayWave(approachSe_, false, 1.0f);
		}
		break;

	case AttackPhase::kAttack:
		pos = Add(pos, enemySpeed_);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			enemySpeed_ = { 0.0f, 1.0f, 0.0f };
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 36.0f, 0.0f, 0.0f };
		}
		break;

	case AttackPhase::kLingering:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;

			Vector3 rot = object3d_->GetRotate();
			turnFirstRotationY = rot.y;
			turnTimer_ = kTimeTurn;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Needle (針)
// ---------------------------------------------------------
void Enemy::BehaviorNeedleInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 50.0f;
	attackLingeringTimer_ = 40.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 0.0f, 20.0f, 0.0f };
	t = 0.0f;

	direction_ = Direction::kFront;
	Vector3 rot = object3d_->GetRotate();
	turnFirstRotationY = rot.y;
	turnTimer_ = kTimeTurn;
	TDEngine::GetAudioManager()->SoundPlayWave(needleSe_, false, 1.0f);
}

void Enemy::BehaviorNeedleUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			TDEngine::GetAudioManager()->SoundPlayWave(needleSe_, false, 1.0f);
			for (int32_t i = 0; i < kNeedleCount; ++i) {
				Needle* needle = new Needle();
				needle->Initialize(pos, needleRotates_[i]);
				needles_.push_back(needle);
			}
		}
		break;

	case AttackPhase::kAttack:
		for (Needle* needle : needles_) needle->Update();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Needle* needle : needles_) delete needle;
			needles_.clear();
		}
		break;

	case AttackPhase::kLingering:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			Vector3 rot = object3d_->GetRotate();
			turnFirstRotationY = rot.y;
			turnTimer_ = kTimeTurn;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Thunder (雷)
// ---------------------------------------------------------
void Enemy::BehaviorThunderInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 200.0f;
	attackLingeringTimer_ = 40.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 36.0f, 0.0f, 0.0f };
	t = 0.0f;
}

void Enemy::BehaviorThunderUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
		}
		break;

	case AttackPhase::kAttack:
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Thunder* thunder : thunders_) delete thunder;
			thunders_.clear();
		}

		if (attackParameter_ % 50 == 1) {
			size_t idx = attackParameter_ / 50;
			if (idx < thunderPositions_.size()) {
				Thunder* thunder = new Thunder();
				thunder->Initialize(thunderPositions_[idx]);
				thunders_.push_back(thunder);
				TDEngine::GetAudioManager()->SoundPlayWave(thunderSe_, false, 1.0f);
			}
		}

		for (Thunder* thunder : thunders_) thunder->Update();
		break;

	case AttackPhase::kLingering:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Punch (連続パンチ)
// ---------------------------------------------------------
void Enemy::BehaviorPunchInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 200.0f;
	attackLingeringTimer_ = 40.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 24.0f, -5.0f, 0.0f };
	t = 0.0f;
}

void Enemy::BehaviorPunchUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			initPos_ = { 22.0f, -5.0f, 0.0f };
			t = 0.0f;

			for (int32_t i = 0; i < kPunchCount; ++i) {
				Punch* punch = new Punch();
				punchPositions_[i] = pos;
				punchPositions_[i].x -= 5.0f * i;
				punchPositions_[i].y += 1.0f - (2.0f * i);
				punch->Initialize(punchPositions_[i], i);
				punches_.push_back(punch);
			}
		}
		break;

	case AttackPhase::kAttack:
		t += 0.025f;
		if (t >= 1.0f) {
			initPos_.x = pos.x - 2.0f;
			t = 0.0f;
		} else {
			pos = EaseIn(t, pos, initPos_);
			object3d_->SetTranslate(pos);
		}

		for (Punch* punch : punches_) punch->Update();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Punch* punch : punches_) delete punch;
			punches_.clear();
		}
		break;

	case AttackPhase::kLingering:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Root (通常行動)
// ---------------------------------------------------------
void Enemy::BehaviorRootInitialize() {
	attackParameter_ = 0;
	attackAfterTimer_ = 20.0f;
}

void Enemy::BehaviorRootUpdate() {
	attackParameter_++;

	if (attackParameter_ >= attackAfterTimer_) {
		randomValue = static_cast<int>(rand_->GetRandom());

		if (static_cast<Behavior>(randomValue) == preBehavior_ ||
			static_cast<Behavior>(randomValue) == prePreBehavior_) {
			return;
		}

		if (isChangeStart_ && !isChanged_) {
			behaviorRequest_ = Behavior::kChange;
			return;
		}

		if (isChanged_) {
			if (hp_ <= halfHp_) {
				if (!canUseThunder_) {
					if (randomValue == static_cast<int>(Behavior::kThunder)) return;
				}
				if (randomValue == static_cast<int>(Behavior::kNeedle) ||
					randomValue == static_cast<int>(Behavior::kThunder) ||
					randomValue == static_cast<int>(Behavior::kPunch) ||
					randomValue == static_cast<int>(Behavior::kBeam)) {
					behaviorRequest_ = static_cast<Behavior>(randomValue);
					return;
				}
			}
		} else {
			if (randomValue == static_cast<int>(Behavior::kApproach) ||
				randomValue == static_cast<int>(Behavior::kBound) ||
				randomValue == static_cast<int>(Behavior::kRound)) {
				behaviorRequest_ = static_cast<Behavior>(randomValue);
				return;
			}
		}
	}
}

// ---------------------------------------------------------
// Behavior: Death (死亡演出)
// ---------------------------------------------------------
void Enemy::BehaviorDeathInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 75.0f;
	attackLingeringTimer_ = 75.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 20.0f, 0.0f, 0.0f };
	enemyRotate_ = { 1.0f, 1.0f, 1.0f };
	t = 0.0f;
	TDEngine::GetAudioManager()->SoundPlayWave(deathSe_, false, 1.0f);
}

void Enemy::BehaviorDeathUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();
	Vector3 rot = object3d_->GetRotate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		rot = Lerp(rot, enemyRotate_, t);
		object3d_->SetRotate(rot);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
		}
		break;

	case AttackPhase::kAttack:
		if (attackParameter_ % 25 == 1) {
			size_t idx = attackParameter_ / 25;
			if (idx < deathExRotates_.size()) {
				DeathEx* deathEx = new DeathEx();
				deathEx->Initialize(pos, deathExRotates_[idx]);
				deathExs_.push_back(deathEx);
			}
		}

		for (DeathEx* deathEx : deathExs_) deathEx->Update();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			for (DeathEx* deathEx : deathExs_) delete deathEx;
			deathExs_.clear();
		}
		break;

	case AttackPhase::kLingering:
		if (attackParameter_ % 15 == 1) {
			EnemyDeathParticle* deathParticle = new EnemyDeathParticle();
			Vector3 pPos = pos;
			pPos.x += static_cast<float>(rand_->GetRandom()) - 4.0f;
			pPos.y -= static_cast<float>(rand_->GetRandom()) - 4.0f;
			pPos.z -= 5.0f;
			deathParticle->Initialize(pPos);
			deathParticles_.push_back(deathParticle);
		}

		for (EnemyDeathParticle* deathParticle : deathParticles_) deathParticle->Update();

		if (color_.w > targetAlpha_) {
			t += 0.001f;
			color_.w = Lerp(color_.w, targetAlpha_, t);
			object3d_->SetColor(color_);
		}

		if (attackParameter_ >= attackLingeringTimer_) {
			isDead_ = true;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Change (形態変化)
// ---------------------------------------------------------
void Enemy::BehaviorChangeInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 115.0f;
	attackLingeringTimer_ = 10.0f;
	attackAfterTimer_ = 10.0f;
	changeColorTimer_ = 0.5f;
	blinkSpeed_ = 0.2f;
	initPos_ = { 20.0f, 0.0f, 0.0f };
	enemyRotate_ = { 1.0f, 1.0f, 1.0f };
	t = 0.0f;
	TDEngine::GetAudioManager()->SoundPlayWave(changeSe_, false, 1.0f);
}

void Enemy::BehaviorChangeUpdate() {
	attackParameter_++;
	Vector3 pos = object3d_->GetTranslate();

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default:
		t += 0.01f;
		pos = Lerp(pos, initPos_, t);
		object3d_->SetTranslate(pos);

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			t = 0.0f;
		}
		break;

	case AttackPhase::kAttack: {
		float rotate = (float)attackParameter_ / attackRushTimer_;
		Vector3 rot = object3d_->GetRotate();
		rot.y = EaseIn(rotate, 2.0f, std::numbers::pi_v<float> *2.0f);
		object3d_->SetRotate(rot);

		t += 0.02f;
		Vector3 scale = object3d_->GetScale();
		scale = Lerp(scale, changeScale_, t);
		object3d_->SetScale(scale);

		float time = attackParameter_ * changeColorTimer_;
		color_.x = (std::sin(time + 0.0f) * 0.5f) + 0.5f;
		color_.y = (std::sin(time + 2.094f) * 0.5f) + 0.5f;
		color_.z = (std::sin(time + 4.189f) * 0.5f) + 0.5f;

		targetAlpha_ = (std::sin(time * blinkSpeed_) * 0.5f) + 0.5f * alphaRange_ + minAlpha_;
		color_.w = EaseOut(t, color_.w, targetAlpha_);
		object3d_->SetColor(color_);

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			object3d_->SetRotate({ 0.0f, 0.0f, 0.0f });
			color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
			object3d_->SetColor(color_);
		}
		break;
	}

	case AttackPhase::kLingering:
		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			isChanged_ = true;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
		break;
	}
}

// ---------------------------------------------------------
// Behavior: Start (開始演出)
// ---------------------------------------------------------
void Enemy::BehaviorStartInitialize() {
	initPos_ = { 20.0f, 0.0f, 0.0f };
	t = 0.0f;
	direction_ = Direction::kLeft;

	Vector3 rot = object3d_->GetRotate();
	turnFirstRotationY = rot.y;
	turnTimer_ = kTimeTurn;
}

void Enemy::BehaviorStartUpdate() {
	t += 0.01f;
	Vector3 pos = object3d_->GetTranslate();
	pos = Lerp(pos, initPos_, t);
	object3d_->SetTranslate(pos);

	if (pos.y <= initPos_.y) {
		behaviorRequest_ = Behavior::kRoot;
		attackPhase_ = AttackPhase::kReservoir;

		if (isUnknown_) {
			behaviorRequest_ = Behavior::kUnknown;
		} else {
			behaviorRequest_ = Behavior::kRoot;
		}
	}
}

// ---------------------------------------------------------
// その他ユーティリティ
// ---------------------------------------------------------
Vector3 Enemy::GetWorldPosition() {
	/*if (object3d_) return object3d_->GetTranslate();
	return { 0,0,0 };*/

	Matrix4x4 world = MyMath::MakeAffineMatrix(object3d_->GetScale(),
                                               object3d_->GetRotate(),
                                               object3d_->GetTranslate());

    // ワールド行列の平行移動成分が「世界座標」
    return Vector3{world.m[3][0], world.m[3][1], world.m[3][2]};
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	return {
		{worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f},
		{worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}
	};
}

void Enemy::OnCollision(const Player* player) {
	if (behavior_ == Behavior::kDeath || isHit_) return;
	isCollisionDisabled_ = true;
	PlayerHitDamage(*player);
}

void Enemy::PlayerHitDamage(const Player& player) {
	if (behavior_ == Behavior::kDeath || behavior_ == Behavior::kChange || behavior_ == Behavior::kStart) return;

	if (!isChangeStart_) {
		if (hp_ <= halfHp_) {
			isChangeStart_ = true;
			return;
		}
	}
	if (hp_ <= 0) {
		behaviorRequest_ = Behavior::kDeath;
		isCollisionDisabled_ = true;
		return;
	}

	hp_ -= player.GetScale().x * 10.0f; // プレイヤーの攻撃力(仮)
	isHit_ = true;
}

void Enemy::BombHitDamage() {
	hp_ -= 10;
	isHit_ = true;

	if (!isChangeStart_) {
		if (hp_ <= halfHp_) {
			behaviorRequest_ = Behavior::kChange;
			isChangeStart_ = true;
		}
	}
	if (hp_ <= 0) {
		behaviorRequest_ = Behavior::kDeath;
	}
}

void Enemy::HitTimer() {
	hitTimer_--;
	if (hitTimer_ <= 0) {
		isHit_ = false;
		hitTimer_ = hitTimerMax_;
	}
}

void Enemy::OnCollision(const ChainBomb* chainBomb) {
	if (chainBomb->IsDestroy()) return;
	if (!chainBomb->IsExplode()) return;
	BombHitDamage();
}