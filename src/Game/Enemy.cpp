#include "Enemy.h"
#include "ChainBomb.h"
#include "Player.h"
#include <cmath>
#include <algorithm>

using namespace TDEngine;
using namespace MyMath; // TDEngineの数学関数を使用

// ヘルパー関数: TDEngineのWorldTransformにはEaseInなどがないため自前で定義
float EaseInFloat(float t, float start, float end) {
	return start + (end - start) * (t * t);
}
float EaseOutFloat(float t, float start, float end) {
	return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t));
}
// 線形補間(Vector3)
Vector3 LerpVec3(const Vector3& v1, const Vector3& v2, float t) {
	return {
		v1.x + (v2.x - v1.x) * t,
		v1.y + (v2.y - v1.y) * t,
		v1.z + (v2.z - v1.z) * t
	};
}
// 線形補間(Vector3 - 角度用)
Vector3 LerpRotate(const Vector3& v1, const Vector3& v2, float t) {
	// 単純なLerpで実装 (必要なら最短経路補間などを追加)
	return LerpVec3(v1, v2, t);
}
// 加算
Vector3 AddVec3(const Vector3& v1, const Vector3& v2) {
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}


// 初期化関数
void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	// nullポインタチェック
	assert(model);

	// 引数をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ビームのモデル生成
	modelBeam_ = Model::CreateFromOBJ("beam", true);
	// 針のモデル生成
	modelNeedle_ = Model::CreateFromOBJ("needle", true);
	// 雷のモデル生成
	modelThunder_ = Model::CreateFromOBJ("Thunder", true);
	// パンチのモデル生成
	modelPunch_ = Model::CreateFromOBJ("punch", true);
	// デスエフェクトのモデル生成
	modelDeathEx_ = Model::CreateFromOBJ("deathEx", true);
	// デスパーティクルのモデル生成
	modelDeathParticle_ = Model::CreateFromOBJ("deathParticle", true);

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation = position; // translation_ -> translation
	worldTransform_.scale = originalScale_; // scale_ -> scale

	// ワールドトランスフォーム更新
	worldTransform_.UpdateMatrix();

	// モデルのアルファ値を設定
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 敵の行動フェーズを決定
	rand_ = new Rand();
	rand_->Initialize();
	rand_->RandomInitialize();
	randomValue = static_cast<int>(rand_->GetRandom());
	behaviorRequest_ = Behavior::kStart;

	// 初期位置調整（Initialize内）
	worldTransform_.translation.x = 2.0f;
	worldTransform_.translation.y = 2.0f;
};

// 更新関数
void Enemy::Update() {

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		if (turnTimer_ <= 1.0f) {
			turnTimer_ += 0.01f;
		}
		// 旋回制御
		float destinationRotationYTable[] = { std::numbers::pi_v<float> *3.0f / 2.0f, std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> };

		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(direction_)];
		// 自キャラの角度を設定する (rotation_ -> rotation)
		worldTransform_.rotation.y = EaseInFloat(turnTimer_, worldTransform_.rotation.y, destinationRotationY);
	}

	if (isHit_) {
		// ヒットタイマー処理
		HitTimer();
	}

	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振る舞いを変更する
		behavior_ = behaviorRequest_;
		// 各振る舞いごとの初期化を実行
		switch (behavior_) {
		case Enemy::Behavior::kRoot: BehaviorRootInitialize(); break;
		case Enemy::Behavior::kBound: BehaviorBoundInitialize(); break;
		case Enemy::Behavior::kRound: BehaviorRoundInitialize(); break;
		case Enemy::Behavior::kBeam: BehaviorBeamInitialize(); break;
		case Enemy::Behavior::kApproach: BehaviorApproachInitialize(); break;
		case Enemy::Behavior::kNeedle: BehaviorNeedleInitialize(); break;
		case Enemy::Behavior::kThunder: BehaviorThunderInitialize(); break;
		case Enemy::Behavior::kPunch: BehaviorPunchInitialize(); break;
		case Enemy::Behavior::kDeath: BehaviorDeathInitialize(); break;
		case Enemy::Behavior::kChange: BehaviorChangeInitialize(); break;
		case Enemy::Behavior::kStart: BehaviorStartInitialize(); break;
		}
		// 振る舞いリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Enemy::Behavior::kRoot: BehaviorRootUpdate(); break;
	case Enemy::Behavior::kBound: BehaviorBoundUpdate(); break;
	case Enemy::Behavior::kRound: BehaviorRoundUpdate(); break;
	case Enemy::Behavior::kBeam: BehaviorBeamUpdate(); break;
	case Enemy::Behavior::kApproach: BehaviorApproachUpdate(); break;
	case Enemy::Behavior::kNeedle: BehaviorNeedleUpdate(); break;
	case Enemy::Behavior::kThunder: BehaviorThunderUpdate(); break;
	case Enemy::Behavior::kPunch: BehaviorPunchUpdate(); break;
	case Enemy::Behavior::kDeath: BehaviorDeathUpdate(); break;
	case Enemy::Behavior::kChange: BehaviorChangeUpdate(); break;
	case Enemy::Behavior::kStart: BehaviorStartUpdate(); break;
	}
}

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

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			enemySpeed_ = { -0.5f, 2.0f, 0.0f };
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		if (attackCount_ >= 3) {
			if (attackParameter_ >= attackRushTimer_) {
				attackPhase_ = AttackPhase::kLingering;
				attackParameter_ = 0;
				t = 0.0f;
			}
		}
		else {
			worldTransform_.translation = AddVec3(worldTransform_.translation, enemySpeed_);
			worldTransform_.UpdateMatrix();
			if (enemySpeed_.y <= -2.0f) {
				if (attackAfterTimer_ >= 0.0f) {
					attackAfterTimer_--;
					worldTransform_.translation.y = -20.0f;
				}
				else {
					if (player_->GetWorldTransform().translation.x >= worldTransform_.translation.x) {
						enemySpeed_.x = 0.5f;
						direction_ = Direction::kRight;
						turnFirstRotationY = worldTransform_.rotation.y;
						turnTimer_ = kTimeTurn;
					}
					else {
						enemySpeed_.x = -0.5f;
						direction_ = Direction::kLeft;
						turnFirstRotationY = worldTransform_.rotation.y;
						turnTimer_ = kTimeTurn;
					}
					enemySpeed_.y = 2.0f;
					attackCount_++;
					attackAfterTimer_ = 10.0f;
				}
			}
			else {
				enemySpeed_.y -= enemySpeedDecay_.y;
			}
		}
	} break;
	case Enemy::AttackPhase::kLingering: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			turnFirstRotationY = worldTransform_.rotation.y;
			turnTimer_ = kTimeTurn;
		}
	} break;
	}
}

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

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			enemySpeed_ = { -1.8f, -2.0f, 0.0f };
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		if (attackCount_ >= 2) {
			if (attackParameter_ >= attackRushTimer_) {
				attackPhase_ = AttackPhase::kLingering;
				attackParameter_ = 0;
				t = 0.0f;
			}
		}
		else {
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
				turnFirstRotationY = worldTransform_.rotation.y;
				turnTimer_ = kTimeTurn;
			}
			else {
				enemySpeed_.y += enemySpeedDecay_.y;
			}
			worldTransform_.translation = AddVec3(worldTransform_.translation, enemySpeed_);
			worldTransform_.UpdateMatrix();
		}
	} break;
	case Enemy::AttackPhase::kLingering: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			turnFirstRotationY = worldTransform_.rotation.y;
			turnTimer_ = kTimeTurn;
		}
	} break;
	}
}

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

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			beam_ = new Beam();
			beam_->Initialize(modelBeam_, camera_, worldTransform_.translation);
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		beam_->Update();
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			delete beam_;
			beam_ = nullptr;
		}
	} break;
	case Enemy::AttackPhase::kLingering: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
	} break;
	}
}

void Enemy::BehaviorApproachInitialize() {
	attackParameter_ = 0;
	enemySpeed_ = { 0.0f, 0.0f, 1.0f };
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 50.0f;
	attackLingeringTimer_ = 40.0f;
	initPos_ = { player_->GetWorldPosition().x, player_->GetWorldPosition().y, 20.0f };
	t = 0.0f;
}

void Enemy::BehaviorApproachUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			enemySpeed_ = { 0.0f, 0.0f, -2.0f };
			attackParameter_ = 0;
			direction_ = Direction::kFront;
			turnFirstRotationY = worldTransform_.rotation.y;
			turnTimer_ = kTimeTurn;
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		worldTransform_.translation = AddVec3(worldTransform_.translation, enemySpeed_);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			enemySpeed_ = { 0.0f, 1.0f, 0.0f };
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 36.0f, 0.0f, 0.0f };
		}
	} break;
	case Enemy::AttackPhase::kLingering: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			turnFirstRotationY = worldTransform_.rotation.y;
			turnTimer_ = kTimeTurn;
		}
	} break;
	}
}

void Enemy::BehaviorNeedleInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 50.0f;
	attackLingeringTimer_ = 40.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 0.0f, 20.0f, 0.0f };
	t = 0.0f;
	direction_ = Direction::kFront;
	turnFirstRotationY = worldTransform_.rotation.y;
	turnTimer_ = kTimeTurn;
}

void Enemy::BehaviorNeedleUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			for (int32_t i = 0; i < kNeedleCount; ++i) {
				Needle* needle = new Needle();
				needle->Initialize(modelNeedle_, camera_, worldTransform_.translation, needleRotates_[i]);
				needles_.push_back(needle);
			}
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		for (Needle* needle : needles_) {
			needle->Update();
		}
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Needle* needle : needles_) {
				delete needle;
			}
			needles_.clear();
		}
	} break;
	case Enemy::AttackPhase::kLingering: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			turnFirstRotationY = worldTransform_.rotation.y;
			turnTimer_ = kTimeTurn;
		}
	} break;
	}
}

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

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Thunder* thunder : thunders_) {
				delete thunder;
			}
			thunders_.clear();
		}

		if (attackParameter_ % 50 == 1) {
			Thunder* thunder = new Thunder();
			thunder->Initialize(modelThunder_, camera_, thunderPositions_[attackParameter_ / 50]);
			thunders_.push_back(thunder);
		}

		for (Thunder* thunder : thunders_) {
			thunder->Update();
		}
		worldTransform_.UpdateMatrix();
	} break;
	case Enemy::AttackPhase::kLingering: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
	} break;
	}
}

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

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			initPos_ = { 22.0f, -5.0f, 0.0f };
			t = 0.0f;

			for (int32_t i = 0; i < kPunchCount; ++i) {
				Punch* punch = new Punch();
				punchPositions_[i] = worldTransform_.translation;
				punchPositions_[i].x -= 5.0f * i;
				punchPositions_[i].y += 1.0f - (2.0f * i);
				punch->Initialize(modelPunch_, camera_, punchPositions_[i], i);
				punches_.push_back(punch);
			}
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		t += 0.025f;
		if (t >= 1.0f) {
			initPos_.x = worldTransform_.translation.x - 2.0f;
			t = 0.0f;
		}
		else {
			worldTransform_.translation = EaseInFloat(t, worldTransform_.translation.x, initPos_.x) == 0 ? worldTransform_.translation : LerpVec3(worldTransform_.translation, initPos_, t * t);
			// 簡易的にLerpVec3を使用
			worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t * t);
		}

		for (Punch* punch : punches_) {
			punch->Update();
		}
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Punch* punch : punches_) {
				delete punch;
			}
			punches_.clear();
		}
	} break;
	case Enemy::AttackPhase::kLingering: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
	} break;
	}
}

void Enemy::BehaviorRootInitialize() {
	attackParameter_ = 0;
	attackAfterTimer_ = 20.0f;
}

void Enemy::BehaviorRootUpdate() {
	attackParameter_++;
	worldTransform_.UpdateMatrix();

	if (attackParameter_ >= attackAfterTimer_) {
		randomValue = static_cast<int>(rand_->GetRandom());

		if (static_cast<Behavior>(randomValue) == preBehavior_ || static_cast<Behavior>(randomValue) == prePreBehavior_) {
			return;
		}

		if (isChangeStart_ && !isChanged_) {
			behaviorRequest_ = Behavior::kChange;
			return;
		}

		if (isChanged_) {
			if (hp_ <= halfHp_) {
				if (!canUseThunder_) {
					if (randomValue == static_cast<int>(Behavior::kThunder)) {
						return;
					}
				}
				if (randomValue == static_cast<int>(Behavior::kNeedle) || randomValue == static_cast<int>(Behavior::kThunder) || randomValue == static_cast<int>(Behavior::kPunch) ||
					randomValue == static_cast<int>(Behavior::kBeam)) {
					behaviorRequest_ = static_cast<Behavior>(randomValue);
					return;
				}
			}
		}
		else {
			if (randomValue == static_cast<int>(Behavior::kApproach) || randomValue == static_cast<int>(Behavior::kBound) || randomValue == static_cast<int>(Behavior::kRound)) {
				behaviorRequest_ = static_cast<Behavior>(randomValue);
				return;
			}
		}
	}
}

void Enemy::BehaviorDeathInitialize() {
	attackParameter_ = 0;
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 75.0f;
	attackLingeringTimer_ = 75.0f;
	attackAfterTimer_ = 10.0f;
	initPos_ = { 20.0f, 0.0f, 0.0f };
	enemyRotate_ = { 1.0f, 1.0f, 1.0f };
	t = 0.0f;
}

void Enemy::BehaviorDeathUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.rotation = LerpRotate(worldTransform_.rotation, enemyRotate_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		if (attackParameter_ % 25 == 1) {
			DeathEx* deathEx = new DeathEx();
			deathEx->Initialize(modelDeathEx_, camera_, worldTransform_.translation, deathExRotates_[attackParameter_ / 25]);
			deathExs_.push_back(deathEx);
		}
		for (DeathEx* deathEx : deathExs_) {
			deathEx->Update();
		}
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			for (DeathEx* deathEx : deathExs_) {
				delete deathEx;
			}
			deathExs_.clear();
		}
	} break;
	case Enemy::AttackPhase::kLingering: {
		if (attackParameter_ % 15 == 1) {
			EnemyDeathParticle* deathParticle = new EnemyDeathParticle();
			Vector3 pos = worldTransform_.translation;
			pos.x += static_cast<float>(rand_->GetRandom()) - 4.0f;
			pos.y -= static_cast<float>(rand_->GetRandom()) - 4.0f;
			pos.z -= 5.0f;
			deathParticle->Initialize(modelDeathParticle_, camera_, pos);
			deathParticles_.push_back(deathParticle);
		}
		for (EnemyDeathParticle* deathParticle : deathParticles_) {
			deathParticle->Update();
		}

		if (color_.w > targetAlpha_) {
			t += 0.001f;
			color_.w = EaseOutFloat(t, color_.w, targetAlpha_);
			// objectColor_.SetColor(color_); // 削除
		}
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackLingeringTimer_) {
			isDead_ = true;
		}
	} break;
	}
}

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
}

void Enemy::BehaviorStartInitialize() {
	initPos_ = { 20.0f, 0.0f, 0.0f };
	t = 0.0f;
	direction_ = Direction::kLeft;
	turnFirstRotationY = worldTransform_.rotation.y;
	turnTimer_ = kTimeTurn;
}

void Enemy::BehaviorStartUpdate() {
	t += 0.01f;
	worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
	worldTransform_.UpdateMatrix();

	if (worldTransform_.translation.y <= initPos_.y) {
		behaviorRequest_ = Behavior::kRoot;
		attackPhase_ = AttackPhase::kReservoir;
		if (isUnknown_) {
			behaviorRequest_ = Behavior::kUnknown;
		}
		else {
			behaviorRequest_ = Behavior::kRoot;
		}
	}
}

void Enemy::BehaviorChangeUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		t += 0.01f;
		worldTransform_.translation = LerpVec3(worldTransform_.translation, initPos_, t);
		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			t = 0.0f;
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		float rotate = attackParameter_ / attackRushTimer_;
		worldTransform_.rotation.y = EaseInFloat(rotate, 2.0f, std::numbers::pi_v<float> *2.0f);

		t += 0.02f;
		worldTransform_.scale = LerpVec3(worldTransform_.scale, changeScale_, t);

		float time = attackParameter_ * changeColorTimer_;
		color_.x = (std::sin(time + 0.0f) * 0.5f) + 0.5f;
		color_.y = (std::sin(time + 2.094f) * 0.5f) + 0.5f;
		color_.z = (std::sin(time + 4.189f) * 0.5f) + 0.5f;
		targetAlpha_ = (std::sin(time * blinkSpeed_) * 0.5f) + 0.5f * alphaRange_ + minAlpha_;
		color_.w = EaseOutFloat(t, color_.w, targetAlpha_);
		// objectColor_.SetColor(color_); // 削除

		worldTransform_.UpdateMatrix();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t = 0.0f;
			worldTransform_.rotation = { 0.0f, 0.0f, 0.0f };
			color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
			// objectColor_.SetColor(color_); // 削除
		}
	} break;
	case Enemy::AttackPhase::kLingering: {
		worldTransform_.UpdateMatrix();
		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			isChanged_ = true;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
	} break;
	}
}

// 描画関数
void Enemy::Draw() {

	if (!isDead_) {
		// アルファ値をモデルに反映 (点滅などはしないが、透明度フェードはこれで動く)
		if (model_) {
			model_->SetAlpha(color_.w);
			model_->Draw(worldTransform_, *camera_);
		}

		if (beam_) beam_->Draw();

		for (Needle* needle : needles_) {
			if (needle) needle->Draw();
		}
		for (Thunder* thunder : thunders_) {
			if (thunder) thunder->Draw();
		}
		for (Punch* punch : punches_) {
			if (punch) punch->Draw();
		}
		for (DeathEx* deathEx : deathExs_) {
			if (deathEx) deathEx->Draw();
		}
		for (EnemyDeathParticle* deathParticle : deathParticles_) {
			if (deathParticle) deathParticle->Draw();
		}
	}
}

TDEngine::Vector3 Enemy::GetWorldPosition() {
	return worldTransform_.translation;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };
	return aabb;
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

	hp_ -= player.GetScale().x * 10.0f;
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

Enemy::~Enemy() {
	if (beam_) delete beam_;
	for (Needle* needle : needles_) delete needle;
	for (Thunder* thunder : thunders_) delete thunder;
	for (Punch* punch : punches_) delete punch;
	for (DeathEx* deathEx : deathExs_) delete deathEx;
	for (EnemyDeathParticle* deathParticle : deathParticles_) delete deathParticle;
}

void Enemy::OnCollision(const ChainBomb* chainBomb) {
	if (chainBomb->IsDestroy()) return;
	if (!chainBomb->IsExplode()) return;
	BombHitDamage();
}