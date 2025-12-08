#include "Enemy.h"
#include "Player.h"
#include "ChainBomb.h"
#include "Beam.h"
#include "Needle.h"
#include "Thunder.h"
#include "Punch.h"
#include "DeathEx.h"
#include "EnemyDeathParticle.h"
#include "Rand.h"
#include "Model.h"

#include <cmath>
#include <algorithm>
#include <numbers>

using namespace MyMath;

Enemy::~Enemy() {
	if (beam_) delete beam_;
	for (auto p : needles_) delete p;
	for (auto p : thunders_) delete p;
	for (auto p : punches_) delete p;
	for (auto p : deathExs_) delete p;
	for (auto p : deathParticles_) delete p;
	if (rand_) delete rand_;
	if (object3d_) delete object3d_;
}

void Enemy::Initialize(const Vector3& position) {
	// Object3d生成	
	std::string path = "./Resources/Enemy/Enemy.obj";
	Model::LoadFromOBJ(path);
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);
	object3d_->SetTranslate(position);
	object3d_->SetScale(originalScale_);
	object3d_->SetColor({ 1, 1, 1, 1 });

	// パラメータ初期化
	hp_ = 100.0f; // 仮置き
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 乱数
	rand_ = new Rand();
	rand_->Initialize();
	rand_->RandomInitialize();
	randomValue_ = static_cast<int>(rand_->GetRandom());
	behaviorRequest_ = Behavior::kStart;

	// 攻撃データの初期化 (元コードのコンストラクタ/メンバ初期化相当)
	needleRotates_ = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 3.14159f / 4.2f},
		{0.0f, 0.0f, 3.14159f / 2.0f},
		{0.0f, 0.0f, 3.14159f / 1.3f}
	};
	// ... thunderPositions_, punchPositions_ も同様に初期化 ...
	thunderPositions_ = {
		{-28.0f, 20.0f, 0.0f}, {-13.0f, 20.0f, 0.0f}, {3.0f, 20.0f, 0.0f}, {16.0f, 20.0f, 0.0f}
	};
}

void Enemy::Update() {
	// 旋回制御
	if (turnTimer_ > 0.0f) {
		if (turnTimer_ <= 1.0f) turnTimer_ += 0.01f; // 元コードのロジック踏襲

		float destinationRotationYTable[] = {
			3.14159f * 1.5f, // Left? (元コードのテーブル順序に依存)
			3.14159f * 0.5f,
			3.14159f
		};
		float destRotY = destinationRotationYTable[static_cast<int>(direction_)];

		Vector3 rot = object3d_->GetRotate();
		rot.y = EaseIn(turnTimer_, rot.y, destRotY); // 簡易補間
		object3d_->SetRotate(rot);
	}

	if (isHit_) HitTimer();

	// ステート遷移
	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;
		switch (behavior_) {
		case Behavior::kRoot: BehaviorRootInitialize(); break;
		case Behavior::kBound: BehaviorBoundInitialize(); break;
		case Behavior::kRound: BehaviorRoundInitialize(); break;
		case Behavior::kBeam: BehaviorBeamInitialize(); break;
		case Behavior::kApproach: BehaviorApproachInitialize(); break;
		case Behavior::kNeedle: BehaviorNeedleInitialize(); break;
		case Behavior::kThunder: BehaviorThunderInitialize(); break;
		case Behavior::kPunch: BehaviorPunchInitialize(); break;
		case Behavior::kDeath: BehaviorDeathInitialize(); break;
		case Behavior::kChange: BehaviorChangeInitialize(); break;
		case Behavior::kStart: BehaviorStartInitialize(); break;
		}
		behaviorRequest_ = Behavior::kUnknown;
	}

	// ステート更新実行
	switch (behavior_) {
	case Enemy::Behavior::kRoot:
		// 通常行動の初期化
		BehaviorRootInitialize();
		break;
	case Enemy::Behavior::kBound:
		// 跳ね回るの初期化
		BehaviorBoundInitialize();
		break;
	case Enemy::Behavior::kRound:
		// 往復の初期化
		BehaviorRoundInitialize();
		break;
	case Enemy::Behavior::kBeam:
		// ビーム攻撃の初期化
		BehaviorBeamInitialize();
		break;
	case Enemy::Behavior::kApproach:
		// 接近の初期化
		BehaviorApproachInitialize();
		break;
	case Enemy::Behavior::kNeedle:
		// 針攻撃の初期化
		BehaviorNeedleInitialize();
		break;
	case Enemy::Behavior::kThunder:
		// 雷攻撃の初期化
		BehaviorThunderInitialize();
		break;
	case Enemy::Behavior::kPunch:
		// 連続パンチの初期化
		BehaviorPunchInitialize();
		break;
	case Enemy::Behavior::kDeath:
		// デス演出の初期化
		BehaviorDeathInitialize();
		break;
	case Enemy::Behavior::kChange:
		// 形態変化の初期化
		BehaviorChangeInitialize();
		break;
	case Enemy::Behavior::kStart:
		// 開始演出の初期化
		BehaviorStartInitialize();
		break;
	}

	// 行列更新
	if (object3d_) object3d_->Update();
}

void Enemy::Draw() {
	if (!isDead_) {
		if (isHit_) {
			// 点滅
			if (hitTimer_ % 2 == 0) object3d_->Draw();
		}
		else {
			object3d_->Draw();
		}

		if (beam_) beam_->Draw();
		for (auto p : needles_) p->Draw();
		for (auto p : thunders_) p->Draw();
		for (auto p : punches_) p->Draw();
		for (auto p : deathExs_) p->Draw();
		for (auto p : deathParticles_) p->Draw();
	}
}

void Enemy::BehaviorStartInitialize() {
	initPos_ = { 20.0f, 0.0f, 0.0f };
	t_ = 0.0f;
	direction_ = Direction::kLeft;
	// turnFirstRotationY = ...
}

void Enemy::BehaviorStartUpdate() {
	t_ += 0.01f;
	Vector3 currentPos = object3d_->GetTranslate();
	Vector3 newPos = Lerp(currentPos, initPos_, t_);
	object3d_->SetTranslate(newPos);

	if (newPos.y <= initPos_.y) {
		behaviorRequest_ = isUnknown_ ? Behavior::kUnknown : Behavior::kRoot;
		attackPhase_ = AttackPhase::kReservoir;
	}
}

void Enemy::BehaviorRootInitialize() {
	attackParameter_ = 0;
	attackAfterTimer_ = 20.0f;
}

void Enemy::BehaviorRootUpdate() {
	attackParameter_++;
	if (attackParameter_ >= attackAfterTimer_) {
		// 行動抽選ロジック (元コードの通り)
		randomValue_ = static_cast<int>(rand_->GetRandom());

		// 簡易移植: 条件分岐は元コードを参照して記述
		// ...
		// テスト用に適当な行動へ遷移させる例:
		if (!isChangeStart_ && hp_ <= halfHp_) {
			isChangeStart_ = true;
			behaviorRequest_ = Behavior::kChange;
			return;
		}

		// 抽選結果適用
		// behaviorRequest_ = ...
	}
}

// ... 他のBehavior関数も同様に、
// worldTransform_.translation_ を object3d_->SetTranslate/GetTranslate に、
// worldTransform_.Add などを MyMath::Add に書き換えて実装します。

void Enemy::OnCollision(const Player* player) {
	if (behavior_ == Behavior::kDeath || isHit_) return;
	isCollisionDisabled_ = true;
	PlayerHitDamage(*player);
}

void Enemy::PlayerHitDamage(const Player& player) {
	// ダメージ計算
	hp_ -= player.GetScale().x * 10.0f;
	isHit_ = true;
	hitTimer_ = hitTimerMax_;
	if (hp_ <= 0) behaviorRequest_ = Behavior::kDeath;
}

void Enemy::BombHitDamage() {
	hp_ -= 10.0f;
	isHit_ = true;
	hitTimer_ = hitTimerMax_;
	if (hp_ <= 0) behaviorRequest_ = Behavior::kDeath;
}

void Enemy::HitTimer() {
	hitTimer_--;
	if (hitTimer_ <= 0) {
		isHit_ = false;
		hitTimer_ = hitTimerMax_;
	}
}

Vector3 Enemy::GetWorldPosition() const {
	return object3d_->GetTranslate();
}

AABB Enemy::GetAABB() {
	Vector3 pos = GetWorldPosition();
	AABB aabb;
	aabb.min = { pos.x - kWidth / 2.0f, pos.y - kHeight / 2.0f, pos.z - kWidth / 2.0f };
	aabb.max = { pos.x + kWidth / 2.0f, pos.y + kHeight / 2.0f, pos.z + kWidth / 2.0f };
	return aabb;
}

// ChainBombとの衝突
void Enemy::OnCollision(const ChainBomb* chainBomb) {
	// ChainBomb側のゲッターが必要 (IsExplodeなど)
	// if (!chainBomb->IsExplode()) return; 
	// BombHitDamage();
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
	t_ = 0.0f;
}

void Enemy::BehaviorBoundUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			enemySpeed_ = { -0.5f, 2.0f, 0.0f };
		}
	} break;
	case AttackPhase::kAttack: {
		if (attackCount_ >= 3) {
			if (attackParameter_ >= attackRushTimer_) {
				attackPhase_ = AttackPhase::kLingering;
				attackParameter_ = 0;
				t_ = 0.0f;
			}
		}
		else {
			Vector3 currentPos = object3d_->GetTranslate();
			object3d_->SetTranslate(Add(currentPos, enemySpeed_));

			if (enemySpeed_.y <= -2.0f) {
				if (attackAfterTimer_ >= 0.0f) {
					attackAfterTimer_--;
					Vector3 pos = object3d_->GetTranslate();
					pos.y = -20.0f;
					object3d_->SetTranslate(pos);
				}
				else {
					Vector3 playerPos = player_->GetWorldPosition();
					Vector3 myPos = object3d_->GetTranslate();
					Vector3 rot = object3d_->GetRotate();

					if (playerPos.x >= myPos.x) {
						enemySpeed_.x = 0.5f;
						direction_ = Direction::kRight;
						turnFirstRotationY_ = rot.y;
						turnTimer_ = kTimeTurn;
					}
					else {
						enemySpeed_.x = -0.5f;
						direction_ = Direction::kLeft;
						turnFirstRotationY_ = rot.y;
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
	case AttackPhase::kLingering: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			turnFirstRotationY_ = object3d_->GetRotate().y;
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
	t_ = 0.0f;
}

void Enemy::BehaviorRoundUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			enemySpeed_ = { -1.8f, -2.0f, 0.0f };
		}
	} break;
	case AttackPhase::kAttack: {
		if (attackCount_ >= 2) {
			if (attackParameter_ >= attackRushTimer_) {
				attackPhase_ = AttackPhase::kLingering;
				attackParameter_ = 0;
				t_ = 0.0f;
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
				turnFirstRotationY_ = object3d_->GetRotate().y;
				turnTimer_ = kTimeTurn;
			}
			else {
				enemySpeed_.y += enemySpeedDecay_.y;
			}
			Vector3 currentPos = object3d_->GetTranslate();
			object3d_->SetTranslate(Add(currentPos, enemySpeed_));
		}
	} break;
	case AttackPhase::kLingering: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			turnFirstRotationY_ = object3d_->GetRotate().y;
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
	t_ = 0.0f;
}

void Enemy::BehaviorBeamUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			beam_ = new Beam();
			beam_->Initialize(object3d_->GetTranslate());
		}
	} break;
	case AttackPhase::kAttack: {
		if (beam_) beam_->Update();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t_ = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			delete beam_;
			beam_ = nullptr;
		}
	} break;
	case AttackPhase::kLingering: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
	} break;
	}
}

// エラーが出ていた箇所
void Enemy::BehaviorApproachInitialize() {
	attackParameter_ = 0;
	enemySpeed_ = { 0.0f, 0.0f, 1.0f };
	attackReservoirTimer_ = 40.0f;
	attackRushTimer_ = 50.0f;
	attackLingeringTimer_ = 40.0f;
	Vector3 playerPos = player_->GetWorldPosition();
	initPos_ = { playerPos.x, playerPos.y, 20.0f };
	t_ = 0.0f;
}

void Enemy::BehaviorApproachUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			enemySpeed_ = { 0.0f, 0.0f, -2.0f };
			attackParameter_ = 0;
			direction_ = Direction::kFront;
			turnFirstRotationY_ = object3d_->GetRotate().y;
			turnTimer_ = kTimeTurn;
		}
	} break;
	case AttackPhase::kAttack: {
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Add(currentPos, enemySpeed_));

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			enemySpeed_ = { 0.0f, 1.0f, 0.0f };
			attackParameter_ = 0;
			t_ = 0.0f;
			initPos_ = { 36.0f, 0.0f, 0.0f };
		}
	} break;
	case AttackPhase::kLingering: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			turnFirstRotationY_ = object3d_->GetRotate().y;
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
	t_ = 0.0f;
	direction_ = Direction::kFront;
	turnFirstRotationY_ = object3d_->GetRotate().y;
	turnTimer_ = kTimeTurn;
}

void Enemy::BehaviorNeedleUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			for (size_t i = 0; i < needleRotates_.size(); ++i) { // needleRotates_はInitializeで設定済み
				Needle* needle = new Needle();
				needle->Initialize(object3d_->GetTranslate(), needleRotates_[i]);
				needles_.push_back(needle);
			}
		}
	} break;
	case AttackPhase::kAttack: {
		for (Needle* needle : needles_) {
			needle->Update();
		}

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t_ = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Needle* needle : needles_) {
				delete needle;
			}
			needles_.clear();
		}
	} break;
	case AttackPhase::kLingering: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
			direction_ = Direction::kLeft;
			turnFirstRotationY_ = object3d_->GetRotate().y;
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
	t_ = 0.0f;
}

void Enemy::BehaviorThunderUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
		}
	} break;
	case AttackPhase::kAttack: {
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t_ = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Thunder* thunder : thunders_) delete thunder;
			thunders_.clear();
		}

		if (attackParameter_ % 50 == 1) {
			size_t index = attackParameter_ / 50;
			if (index < thunderPositions_.size()) {
				Thunder* thunder = new Thunder();
				thunder->Initialize(thunderPositions_[index]);
				thunders_.push_back(thunder);
			}
		}

		for (Thunder* thunder : thunders_) {
			thunder->Update();
		}
	} break;
	case AttackPhase::kLingering: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

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
	t_ = 0.0f;
}

void Enemy::BehaviorPunchUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			initPos_ = { 22.0f, -5.0f, 0.0f };
			t_ = 0.0f;

			for (int i = 0; i < 2; ++i) { // kPunchCount = 2
				Punch* punch = new Punch();
				Vector3 pPos = object3d_->GetTranslate();
				pPos.x -= 5.0f * i;
				pPos.y += 1.0f - (2.0f * i);
				punch->Initialize(pPos, i);
				punches_.push_back(punch);
			}
		}
	} break;
	case AttackPhase::kAttack: {
		t_ += 0.025f;
		if (t_ >= 1.0f) {
			initPos_.x = object3d_->GetTranslate().x - 2.0f;
			t_ = 0.0f;
		}
		else {
			Vector3 currentPos = object3d_->GetTranslate();
			object3d_->SetTranslate(EaseIn(t_, currentPos, initPos_));
		}

		for (Punch* punch : punches_) punch->Update();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t_ = 0.0f;
			initPos_ = { 0.0f, 0.0f, 0.0f };
			for (Punch* punch : punches_) delete punch;
			punches_.clear();
		}
	} break;
	case AttackPhase::kLingering: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackLingeringTimer_) {
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			behaviorRequest_ = Behavior::kRoot;
			attackPhase_ = AttackPhase::kReservoir;
		}
	} break;
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
	t_ = 0.0f;
}

void Enemy::BehaviorDeathUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		Vector3 currentRot = object3d_->GetRotate();
		object3d_->SetRotate(Lerp(currentRot, enemyRotate_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
		}
	} break;
	case AttackPhase::kAttack: {
		if (attackParameter_ % 25 == 1) {
			size_t index = attackParameter_ / 25;
			if (index < deathExRotates_.size()) { // deathExRotates_はInitializeで初期化が必要
				DeathEx* deathEx = new DeathEx();
				deathEx->Initialize(object3d_->GetTranslate(), deathExRotates_[index]);
				deathExs_.push_back(deathEx);
			}
		}
		for (DeathEx* deathEx : deathExs_) deathEx->Update();

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t_ = 0.0f;
			for (DeathEx* deathEx : deathExs_) delete deathEx;
			deathExs_.clear();
		}
	} break;
	case AttackPhase::kLingering: {
		if (attackParameter_ % 15 == 1) {
			EnemyDeathParticle* deathParticle = new EnemyDeathParticle();
			Vector3 pos = object3d_->GetTranslate();
			pos.x += static_cast<float>(rand_->GetRandom()) - 4.0f;
			pos.y -= static_cast<float>(rand_->GetRandom()) - 4.0f;
			pos.z -= 5.0f;
			deathParticle->Initialize(pos);
			deathParticles_.push_back(deathParticle);
		}
		for (EnemyDeathParticle* deathParticle : deathParticles_) deathParticle->Update();

		if (color_.w > targetAlpha_) {
			t_ += 0.001f;
			color_.w = Lerp(color_.w, targetAlpha_, t_);
			object3d_->SetColor(color_);
		}

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
	t_ = 0.0f;
}

void Enemy::BehaviorChangeUpdate() {
	attackParameter_++;

	switch (attackPhase_) {
	case AttackPhase::kReservoir:
	default: {
		t_ += 0.01f;
		Vector3 currentPos = object3d_->GetTranslate();
		object3d_->SetTranslate(Lerp(currentPos, initPos_, t_));

		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			t_ = 0.0f;
		}
	} break;
	case AttackPhase::kAttack: {
		float rotate = (float)attackParameter_ / attackRushTimer_;
		Vector3 rot = object3d_->GetRotate();
		rot.y = EaseIn(2.0f, 3.14159f * 2.0f, rotate);
		object3d_->SetRotate(rot);

		t_ += 0.02f;
		object3d_->SetScale(Lerp(object3d_->GetScale(), changeScale_, t_));

		float time = attackParameter_ * changeColorTimer_;
		color_.x = (std::sin(time + 0.0f) * 0.5f) + 0.5f;
		color_.y = (std::sin(time + 2.094f) * 0.5f) + 0.5f;
		color_.z = (std::sin(time + 4.189f) * 0.5f) + 0.5f;
		targetAlpha_ = (std::sin(time * blinkSpeed_) * 0.5f) + 0.5f * alphaRange_ + minAlpha_;
		color_.w = EaseOut(t_, color_.w, targetAlpha_);
		object3d_->SetColor(color_);

		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0;
			t_ = 0.0f;
			object3d_->SetRotate({ 0,0,0 });
			color_ = { 1,1,1,1 };
			object3d_->SetColor(color_);
		}
	} break;
	case AttackPhase::kLingering: {
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