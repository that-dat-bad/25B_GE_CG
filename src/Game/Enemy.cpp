#include "Enemy.h"
#include "ChainBomb.h"
#include "Player.h"
using namespace TDEngine;

// 初期化関数
void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	worldTransform_.translation_.x = 2.0f;
	worldTransform_.translation_.y = 2.0f;

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
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = originalScale_;

	// ワールドトランスフォーム更新
	worldTransform_.UpdateWorldMatrix(worldTransform_);

	// モデルのアルファ値を設定
	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// 敵の行動フェーズを決定
	rand_ = new Rand();
	rand_->Initialize();
	rand_->RandomInitialize();
	randomValue = static_cast<int>(rand_->GetRandom());
	behaviorRequest_ = Behavior::kStart;
};
// 更新関数
void Enemy::Update() {

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		if (turnTimer_ <= 1.0f) {
			turnTimer_ += 0.01f;
		}
		// 旋回制御
		float destinationRotationYTable[] = {std::numbers::pi_v<float> * 3.0f / 2.0f, std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float>};

		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(direction_)];
		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = worldTransform_.EaseInFloat(turnTimer_, worldTransform_.rotation_.y, destinationRotationY);
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
		// 振る舞いリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Enemy::Behavior::kRoot:
		// 通常行動の更新
		BehaviorRootUpdate();
		break;
	case Enemy::Behavior::kBound:
		// 跳ね回るの更新
		BehaviorBoundUpdate();
		break;
	case Enemy::Behavior::kRound:
		// 往復の更新
		BehaviorRoundUpdate();
		break;
	case Enemy::Behavior::kBeam:
		// ビーム攻撃の更新
		BehaviorBeamUpdate();
		break;
	case Enemy::Behavior::kApproach:
		// 接近の更新
		BehaviorApproachUpdate();
		break;
	case Enemy::Behavior::kNeedle:
		// 針攻撃の更新
		BehaviorNeedleUpdate();
		break;
	case Enemy::Behavior::kThunder:
		// 雷攻撃の更新
		BehaviorThunderUpdate();
		break;
	case Enemy::Behavior::kPunch:
		// 連続パンチの更新
		BehaviorPunchUpdate();
		break;
	case Enemy::Behavior::kDeath:
		// デス演出の更新
		BehaviorDeathUpdate();
		break;
	case Enemy::Behavior::kChange:
		// 形態変化の更新
		BehaviorChangeUpdate();
		break;
	case Enemy::Behavior::kStart:
		// 開始演出の更新
		BehaviorStartUpdate();
		break;
	}
}

void Enemy::BehaviorBoundInitialize() {
	// カウンター初期化
	attackParameter_ = 0;

	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 50.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 40.0f;

	// 攻撃の後隙タイマー
	attackAfterTimer_ = 10.0f;

	// 初期位置の初期化
	initPos_ = {36.0f, -20.0f, 0.0f};

	// 攻撃カウントの初期化
	attackCount_ = 0;

	// 速度減衰率の初期化
	enemySpeedDecay_ = {0.0f, 0.1f, 0.0f};

	// 線形補間用変数の初期化
	t = 0.0f;
}

void Enemy::BehaviorBoundUpdate() {
	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		// 跳ね回り準備フェーズの処理

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// カウンターをリセット
			attackParameter_ = 0;
			// 跳ね回りフェーズの速度を初期化
			enemySpeed_ = {-0.5f, 2.0f, 0.0f};
		}
	} break;
	case Enemy::AttackPhase::kAttack: {

		if (attackCount_ >= 3) {
			// 余韻動作へ移行
			if (attackParameter_ >= attackRushTimer_) {
				attackPhase_ = AttackPhase::kLingering;
				attackParameter_ = 0; // カウンターをリセット
				// 線形補間用変数リセット
				t = 0.0f;
			}
		} else {
			// 移動(ベクトルを加算)
			worldTransform_.translation_ = worldTransform_.Add(worldTransform_.translation_, enemySpeed_);
			// 　ワールドトランスフォームの更新
			worldTransform_.UpdateWorldMatrix(worldTransform_);
			// 端まで達したら反転
			if (enemySpeed_.y <= -2.0f) {

				if (attackAfterTimer_ >= 0.0f) {
					attackAfterTimer_--;
					worldTransform_.translation_.y = -20.0f;
				} else {
					// プレイヤーの位置で判定
					if (player_->GetWorldTransform().translation_.x >= worldTransform_.translation_.x) {
						// プレイヤーが右側にいるなら右向きに
						enemySpeed_.x = 0.5f;
						direction_ = Direction::kRight;
						turnFirstRotationY = worldTransform_.rotation_.y;
						turnTimer_ = kTimeTurn;
					} else {
						// プレイヤーが左側にいるなら左向きに
						enemySpeed_.x = -0.5f;
						direction_ = Direction::kLeft;
						turnFirstRotationY = worldTransform_.rotation_.y;
						turnTimer_ = kTimeTurn;
					}

					// 上昇速度をリセット
					enemySpeed_.y = 2.0f;
					// 攻撃回数をカウント
					attackCount_++;
					// 後隙タイマーリセット
					attackAfterTimer_ = 10.0f;
				}

			} else {
				// 速度を減衰
				enemySpeed_.y -= enemySpeedDecay_.y;
			}
		}

	} break;

	case Enemy::AttackPhase::kLingering: {

		/// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 行動フェーズを記録
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
			// 攻撃フェーズをリセット
			attackPhase_ = AttackPhase::kReservoir;
			// 向きを左に戻す
			direction_ = Direction::kLeft;
			turnFirstRotationY = worldTransform_.rotation_.y;
			turnTimer_ = kTimeTurn;
		}

	} break;
	}
}

void Enemy::BehaviorRoundInitialize() {
	// カウンター初期化
	attackParameter_ = 0;

	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 100.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 40.0f;

	// 攻撃の後隙タイマー
	attackAfterTimer_ = 10.0f;

	// 初期位置の初期化
	initPos_ = {36.0f, 20.0f, 0.0f};

	// 攻撃カウントの初期化
	attackCount_ = 0;

	// 速度減衰率の初期化
	enemySpeedDecay_ = {0.0f, 0.1f, 0.0f};

	// 線形補間用変数の初期化
	t = 0.0f;
}

void Enemy::BehaviorRoundUpdate() {
	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		// 往復準備フェーズの処理

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// カウンターをリセット
			attackParameter_ = 0;
			// 往復フェーズの速度を初期化
			enemySpeed_ = {-1.8f, -2.0f, 0.0f};
		}
	} break;
	case Enemy::AttackPhase::kAttack: {

		if (attackCount_ >= 2) {
			// 余韻動作へ移行
			if (attackParameter_ >= attackRushTimer_) {
				attackPhase_ = AttackPhase::kLingering;
				attackParameter_ = 0; // カウンターをリセット
				// 線形補間用変数リセット
				t = 0.0f;
			}
		} else {

			// 端まで達したら反転
			if (enemySpeed_.y >= 1.9f) {

				if (attackAfterTimer_ >= 0.0f) {
					attackAfterTimer_--;
					break;
				}

				enemySpeed_.x *= -1.0f; // X方向反転
				// 下降速度をリセット
				enemySpeed_.y = -2.0f;
				// 攻撃回数をカウント
				attackCount_++;
				// 後隙タイマーリセット
				attackAfterTimer_ = 10.0f;
				// 向きを変更
				direction_ = Direction::kRight;
				turnFirstRotationY = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;

			} else {
				// 速度を減衰
				enemySpeed_.y += enemySpeedDecay_.y;
			}
			// 移動(ベクトルを加算)
			worldTransform_.translation_ = worldTransform_.Add(worldTransform_.translation_, enemySpeed_);
			// 　ワールドトランスフォームの更新
			worldTransform_.UpdateWorldMatrix(worldTransform_);
		}
	} break;

	case Enemy::AttackPhase::kLingering: {

		/// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 行動フェーズを記録
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
			// 攻撃フェーズをリセット
			attackPhase_ = AttackPhase::kReservoir;
			// 向きを左に戻す
			direction_ = Direction::kLeft;
			turnFirstRotationY = worldTransform_.rotation_.y;
			turnTimer_ = kTimeTurn;
		}

	} break;
	}
}

void Enemy::BehaviorBeamInitialize() {
	// カウンター初期化
	attackParameter_ = 0;

	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 50.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 40.0f;

	// 攻撃の後隙タイマー
	attackAfterTimer_ = 10.0f;

	// 初期位置の初期化
	initPos_ = {36.0f, -5.0f, 0.0f};

	// 線形補間用変数の初期化
	t = 0.0f;
}

void Enemy::BehaviorBeamUpdate() {
	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		// ビーム準備フェーズの処理

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// カウンターをリセット
			attackParameter_ = 0;
			// ビームを生成
			beam_ = new Beam();
			// ビームの初期化
			beam_->Initialize(modelBeam_, camera_, worldTransform_.translation_);
		}
	} break;
	case Enemy::AttackPhase::kAttack: {

		// ビームの更新処理
		beam_->Update();

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 余韻動作へ移行
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0; // カウンターをリセット
			// 線形補間用変数リセット
			t = 0.0f;
			// 初期位置変更
			initPos_ = {0.0f, 0.0f, 0.0f};
			// ビームを削除
			delete beam_;
			beam_ = nullptr;
		}

	} break;

	case Enemy::AttackPhase::kLingering: {

		/// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 行動フェーズを記録
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
			// 攻撃フェーズをリセット
			attackPhase_ = AttackPhase::kReservoir;
		}

	} break;
	}
}

void Enemy::BehaviorApproachInitialize() {
	// カウンター初期化
	attackParameter_ = 0;
	// 速度初期化
	enemySpeed_ = {0.0f, 0.0f, 1.0f};
	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 50.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 40.0f;

	// 初期位置の初期化
	initPos_ = {player_->GetWorldPosition().x, player_->GetWorldPosition().y, 20.0f};

	// 線形補間用変数の初期化
	t = 0.0f;
}

void Enemy::BehaviorApproachUpdate() {

	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// 接近フェーズの速度
			enemySpeed_ = {0.0f, 0.0f, -2.0f};
			// カウンターをリセット
			attackParameter_ = 0;
			// プレイヤーの向きを正面に
			direction_ = Direction::kFront;
			turnFirstRotationY = worldTransform_.rotation_.y;
			turnTimer_ = kTimeTurn;
		}
	} break;
	case Enemy::AttackPhase::kAttack: {

		// 移動(ベクトルを加算)
		worldTransform_.translation_ = worldTransform_.Add(worldTransform_.translation_, enemySpeed_);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 余韻動作へ移行
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			// 速度を初期化
			enemySpeed_ = {0.0f, 1.0f, 0.0f};
			// カウンターをリセット
			attackParameter_ = 0;
			// 線形補間用変数リセット
			t = 0.0f;
			// 初期位置変更
			initPos_ = {36.0f, 0.0f, 0.0f};
		}

	} break;

	case Enemy::AttackPhase::kLingering: {
		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 行動フェーズを記録
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
			// 攻撃フェーズをリセット
			attackPhase_ = AttackPhase::kReservoir;
			// プレイヤーの向きを元に戻す
			direction_ = Direction::kLeft;
			turnFirstRotationY = worldTransform_.rotation_.y;
			turnTimer_ = kTimeTurn;
		}
	} break;
	}
}

void Enemy::BehaviorNeedleInitialize() {
	// カウンター初期化
	attackParameter_ = 0;

	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 50.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 40.0f;

	// 攻撃の後隙タイマー
	attackAfterTimer_ = 10.0f;

	// 初期位置の初期化
	initPos_ = {0.0f, 20.0f, 0.0f};

	// 線形補間用変数の初期化
	t = 0.0f;

	// プレイヤーの向きを正面に
	direction_ = Direction::kFront;
	turnFirstRotationY = worldTransform_.rotation_.y;
	turnTimer_ = kTimeTurn;
}

void Enemy::BehaviorNeedleUpdate() {
	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		// 針準備フェーズの処理

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// カウンターをリセット
			attackParameter_ = 0;
			// 針を生成
			for (int32_t i = 0; i < kNeedleCount; ++i) {
				Needle* needle = new Needle();
				// 針の初期化
				needle->Initialize(modelNeedle_, camera_, worldTransform_.translation_, needleRotates_[i]);
				needles_.push_back(needle);
			}
		}
	} break;
	case Enemy::AttackPhase::kAttack: {

		// 針の更新処理
		for (Needle* needle : needles_) {
			needle->Update();
		}

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 余韻動作へ移行
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0; // カウンターをリセット
			// 線形補間用変数リセット
			t = 0.0f;
			// 初期位置変更
			initPos_ = {0.0f, 0.0f, 0.0f};
			// 針を削除
			for (Needle* needle : needles_) {
				delete needle;
			}
			needles_.clear();
		}

	} break;

	case Enemy::AttackPhase::kLingering: {

		/// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 行動フェーズを記録
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
			// 攻撃フェーズをリセット
			attackPhase_ = AttackPhase::kReservoir;
			// プレイヤーの向きを元に戻す
			direction_ = Direction::kLeft;
			turnFirstRotationY = worldTransform_.rotation_.y;
			turnTimer_ = kTimeTurn;
		}

	} break;
	}
}

void Enemy::BehaviorThunderInitialize() {

	// カウンター初期化
	attackParameter_ = 0;

	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 200.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 40.0f;

	// 攻撃の後隙タイマー
	attackAfterTimer_ = 10.0f;

	// 初期位置の初期化
	initPos_ = {36.0f, 0.0f, 0.0f};

	// 線形補間用変数の初期化
	t = 0.0f;
}

void Enemy::BehaviorThunderUpdate() {
	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		// 針準備フェーズの処理

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// カウンターをリセット
			attackParameter_ = 0;
		}
	} break;
	case Enemy::AttackPhase::kAttack: {

		// 余韻動作へ移行
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0; // カウンターをリセット
			// 線形補間用変数リセット
			t = 0.0f;
			// 初期位置変更
			initPos_ = {0.0f, 0.0f, 0.0f};
			// 雷を削除
			for (Thunder* thunder : thunders_) {
				delete thunder;
			}
			thunders_.clear();
		}

		if (attackParameter_ % 50 == 1) {
			// 雷を生成
			Thunder* thunder = new Thunder();
			// 雷の初期化
			thunder->Initialize(modelThunder_, camera_, thunderPositions_[attackParameter_ / 50]);
			// 雷を登録する
			thunders_.push_back(thunder);
		}

		// 雷の更新処理
		for (Thunder* thunder : thunders_) {
			thunder->Update();
		}

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

	} break;

	case Enemy::AttackPhase::kLingering: {

		/// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 行動フェーズを記録
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
			// 攻撃フェーズをリセット
			attackPhase_ = AttackPhase::kReservoir;
		}

	} break;
	}
}

void Enemy::BehaviorPunchInitialize() {
	// カウンター初期化
	attackParameter_ = 0;

	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 200.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 40.0f;

	// 攻撃の後隙タイマー
	attackAfterTimer_ = 10.0f;

	// 初期位置の初期化
	initPos_ = {24.0f, -5.0f, 0.0f};

	// 線形補間用変数の初期化
	t = 0.0f;
}

void Enemy::BehaviorPunchUpdate() {
	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {
		// 連続パンチ準備フェーズの処理

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// カウンターをリセット
			attackParameter_ = 0;
			// 初期位置を変更
			initPos_ = {22.0f, -5.0f, 0.0f};
			// tを初期化
			t = 0.0f;

			// パンチを生成
			for (int32_t i = 0; i < kPunchCount; ++i) {
				Punch* punch = new Punch();
				// パンチの位置設定
				punchPositions_[i] = worldTransform_.translation_;
				punchPositions_[i].x -= 5.0f * i;
				punchPositions_[i].y += 1.0f - (2.0f * i);
				// パンチの初期化
				punch->Initialize(modelPunch_, camera_, punchPositions_[i], i);
				punches_.push_back(punch);
			}
		}
	} break;
	case Enemy::AttackPhase::kAttack: {

		// tの値を増加
		t += 0.025f;

		if (t >= 1.0f) {
			// 初期位置を変更
			initPos_.x = worldTransform_.translation_.x - 2.0f;
			// tの値をリセット
			t = 0.0f;

		} else {
			// 初期位置に向かってイージング
			worldTransform_.translation_ = worldTransform_.EaseIn(t, worldTransform_.translation_, initPos_);
		}

		// パンチの更新処理
		for (Punch* punch : punches_) {
			punch->Update();
		}

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 余韻動作へ移行
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			attackParameter_ = 0; // カウンターをリセット
			// 線形補間用変数リセット
			t = 0.0f;
			// 初期位置変更
			initPos_ = {0.0f, 0.0f, 0.0f};
			// パンチを削除
			for (Punch* punch : punches_) {
				delete punch;
			}
			punches_.clear();
		}

	} break;

	case Enemy::AttackPhase::kLingering: {

		/// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 行動フェーズを記録
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
			// 攻撃フェーズをリセット
			attackPhase_ = AttackPhase::kReservoir;
		}

	} break;
	}
}

void Enemy::BehaviorRootInitialize() {
	// カウンター初期化
	attackParameter_ = 0;
	// 攻撃の後隙タイマー
	attackAfterTimer_ = 20.0f;
}

void Enemy::BehaviorRootUpdate() {

	// カウンター増加
	attackParameter_++;

	// 行列を定数バッファに移動
	worldTransform_.UpdateWorldMatrix(worldTransform_);

	if (attackParameter_ >= attackAfterTimer_) {

		// 次の行動フェーズを抽選
		randomValue = static_cast<int>(rand_->GetRandom());

		// 前回や前々回と同じ行動フェーズだったら
		if (static_cast<Behavior>(randomValue) == preBehavior_ || static_cast<Behavior>(randomValue) == prePreBehavior_) {
			// やり直す
			return;
		}

		if (isChangeStart_ && !isChanged_) {
			behaviorRequest_ = Behavior::kChange;
			return;
		}

		// 敵のHPが半分以下なら
		if (isChanged_) {
			if (hp_ <= halfHp_) {

				if (!canUseThunder_) {
					if (randomValue == static_cast<int>(Behavior::kThunder)) {
						return;
					}
				}

				// フェーズを限定する
				if (randomValue == static_cast<int>(Behavior::kNeedle) || randomValue == static_cast<int>(Behavior::kThunder) || randomValue == static_cast<int>(Behavior::kPunch) ||
				    randomValue == static_cast<int>(Behavior::kBeam)) {
					// 　次の行動フェーズを決定
					behaviorRequest_ = static_cast<Behavior>(randomValue);
					return;
				}
			}
		} else {

			// 敵のHPが半分以上なら
			if (randomValue == static_cast<int>(Behavior::kApproach) || randomValue == static_cast<int>(Behavior::kBound) || randomValue == static_cast<int>(Behavior::kRound)) {
				// 　次の行動フェーズを決定
				behaviorRequest_ = static_cast<Behavior>(randomValue);
				return;
			}
		}
	}
}

void Enemy::BehaviorDeathInitialize() {
	// カウンター初期化
	attackParameter_ = 0;

	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 75.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 75.0f;

	// 攻撃の後隙タイマー
	attackAfterTimer_ = 10.0f;

	// 初期位置の初期化
	initPos_ = {20.0f, 0.0f, 0.0f};

	// 敵の角度の初期化
	enemyRotate_ = {1.0f, 1.0f, 1.0f};

	// 線形補間用変数の初期化
	t = 0.0f;
}

void Enemy::BehaviorDeathUpdate() {
	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// ゆっくり俯かせる
		worldTransform_.rotation_ = worldTransform_.Lerp(worldTransform_.rotation_, enemyRotate_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// カウンターをリセット
			attackParameter_ = 0;
		}
	} break;
	case Enemy::AttackPhase::kAttack: {

		// 一定間隔で爆発パーティクル生成
		if (attackParameter_ % 25 == 1) {
			DeathEx* deathEx = new DeathEx();
			// 爆発パーティクルの初期化
			deathEx->Initialize(modelDeathEx_, camera_, worldTransform_.translation_, deathExRotates_[attackParameter_ / 25]);
			// 爆発パーティクルを登録する
			deathExs_.push_back(deathEx);
		}

		// デス爆発パーティクルの更新処理
		for (DeathEx* deathEx : deathExs_) {
			deathEx->Update();
		}

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 余韻動作へ移行
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			// カウンターをリセット
			attackParameter_ = 0;
			// 線形補間用変数リセット
			t = 0.0f;
			// デス爆発パーティクルを消去
			for (DeathEx* deathEx : deathExs_) {
				delete deathEx;
			}
			deathExs_.clear();
		}

	} break;

	case Enemy::AttackPhase::kLingering: {
		// 一定間隔でデスパーティクル生成
		if (attackParameter_ % 15 == 1) {
			EnemyDeathParticle* deathParticle = new EnemyDeathParticle();
			// パーティクルの初期化
			Vector3 pos = worldTransform_.translation_;
			pos.x += static_cast<float>(rand_->GetRandom()) - 4.0f;
			pos.y -= static_cast<float>(rand_->GetRandom()) - 4.0f;
			pos.z -= 5.0f;
			deathParticle->Initialize(modelDeathParticle_, camera_, pos);
			// パーティクルを登録する
			deathParticles_.push_back(deathParticle);
		}

		// パーティクルの更新処理
		for (EnemyDeathParticle* deathParticle : deathParticles_) {
			deathParticle->Update();
		}

		// 敵のモデルのアルファ値を徐々にフェードアウト
		if (color_.w > targetAlpha_) {
			t += 0.001f;
			// モデルのアルファ値をイージング
			color_.w = worldTransform_.LerpFloat(color_.w, targetAlpha_, t);
			objectColor_.SetColor(color_);
		}

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 敵を死亡状態にする
			isDead_ = true;
		}
	} break;
	}
}

void Enemy::BehaviorChangeInitialize() {
	// カウンター初期化
	attackParameter_ = 0;

	// タイマー初期化
	// 予備動作のタイマー
	attackReservoirTimer_ = 40.0f;

	// 攻撃動作のタイマー
	attackRushTimer_ = 115.0f;

	// 余韻動作のタイマー
	attackLingeringTimer_ = 10.0f;

	// 攻撃の後隙タイマー
	attackAfterTimer_ = 10.0f;

	// 色変化タイマー
	changeColorTimer_ = 0.5f;

	// 点滅の速さ
	blinkSpeed_ = 0.2f;

	// 初期位置の初期化
	initPos_ = {20.0f, 0.0f, 0.0f};

	// 敵の角度の初期化
	enemyRotate_ = {1.0f, 1.0f, 1.0f};

	// 線形補間用変数の初期化
	t = 0.0f;
}

void Enemy::BehaviorStartInitialize() {
	// 初期位置の初期化
	initPos_ = {20.0f, 0.0f, 0.0f};

	// 線形補間用変数の初期化
	t = 0.0f;

	// 左向きにする
	direction_ = Direction::kLeft;
	turnFirstRotationY = worldTransform_.rotation_.y;
	turnTimer_ = kTimeTurn;
}

void Enemy::BehaviorStartUpdate() {
	// 初期位置に向かって徐々に近づく
	t += 0.01f;
	worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

	// 　ワールドトランスフォームの更新
	worldTransform_.UpdateWorldMatrix(worldTransform_);

	// 初期位置に到達したら通常行動へ移行
	if (worldTransform_.translation_.y <= initPos_.y) {
		// 振る舞いを通常行動に戻す
		behaviorRequest_ = Behavior::kRoot;
		// 攻撃フェーズをリセット
		attackPhase_ = AttackPhase::kReservoir;

		if (isUnknown_) {
			behaviorRequest_ = Behavior::kUnknown;
		} else {
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
		}

	}
}

void Enemy::BehaviorChangeUpdate() {
	// 予備動作
	attackParameter_++;

	switch (attackPhase_) {
	case Enemy::AttackPhase::kReservoir:
	default: {

		// 初期位置に向かって徐々に近づく
		t += 0.01f;
		worldTransform_.translation_ = worldTransform_.Lerp(worldTransform_.translation_, initPos_, t);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 攻撃動作へ移行
		if (attackParameter_ >= attackReservoirTimer_) {
			attackPhase_ = AttackPhase::kAttack;
			// カウンターをリセット
			attackParameter_ = 0;
			// 線形補間用変数リセット
			t = 0.0f;
		}
	} break;
	case Enemy::AttackPhase::kAttack: {
		// 回転させる
		float rotate = attackParameter_ / attackRushTimer_;
		worldTransform_.rotation_.y = worldTransform_.EaseInFloat(2.0f, std::numbers::pi_v<float> * 2.0f, rotate);

		// 徐々に大きくする
		t += 0.02f;
		worldTransform_.scale_ = worldTransform_.Lerp(worldTransform_.scale_, changeScale_, t);

		// 点滅させる
		float time = attackParameter_ * changeColorTimer_;
		// RGBを設定
		color_.x = (std::sin(time + 0.0f) * 0.5f) + 0.5f;
		color_.y = (std::sin(time + 2.094f) * 0.5f) + 0.5f;
		color_.z = (std::sin(time + 4.189f) * 0.5f) + 0.5f;
		// アルファを設定
		targetAlpha_ = (std::sin(time * blinkSpeed_) * 0.5f) + 0.5f * alphaRange_ + minAlpha_;
		// 色を設定
		color_.w = worldTransform_.EaseOutFloat(t, color_.w, targetAlpha_);
		objectColor_.SetColor(color_);

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		// 余韻動作へ移行
		if (attackParameter_ >= attackRushTimer_) {
			attackPhase_ = AttackPhase::kLingering;
			// カウンターをリセット
			attackParameter_ = 0;
			// 線形補間用変数リセット
			t = 0.0f;
			// 敵の角度をリセット
			worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
			// 色を元に戻す
			color_ = {1.0f, 1.0f, 1.0f, 1.0f};
			objectColor_.SetColor(color_);
		}

	} break;

	case Enemy::AttackPhase::kLingering: {

		// 　ワールドトランスフォームの更新
		worldTransform_.UpdateWorldMatrix(worldTransform_);

		if (attackParameter_ >= attackLingeringTimer_) {
			// 行動フェーズを記録
			prePreBehavior_ = preBehavior_;
			preBehavior_ = behavior_;
			isChanged_ = true;
			// 振る舞いを通常行動に戻す
			behaviorRequest_ = Behavior::kRoot;
			// 攻撃フェーズをリセット
			attackPhase_ = AttackPhase::kReservoir;
		}
	} break;
	}
}

// 描画関数
void Enemy::Draw() {

	if (!isDead_) {
		// 敵モデルの描画
		if (isHit_) {
			// ダメージを受けているなら点滅させる
			if (hitTimer_ % 2 == 0) {
				model_->Draw(worldTransform_, *camera_, &objectColor_);
			}

		} else {

			model_->Draw(worldTransform_, *camera_, &objectColor_);
		}

		// ビームの描画
		if (beam_) {
			beam_->Draw();
		}

		// 針の描画
		for (Needle* needle : needles_) {
			if (needle) {
				needle->Draw();
			}
		}

		// 雷の描画
		for (Thunder* thunder : thunders_) {
			if (thunder) {
				thunder->Draw();
			}
		}

		// パンチの描画
		for (Punch* punch : punches_) {
			if (punch) {
				punch->Draw();
			}
		}

		// デス爆発パーティクルの描画
		for (DeathEx* deathEx : deathExs_) {
			if (deathEx) {
				deathEx->Draw();
			}
		}

		// デスパーティクルの描画
		for (EnemyDeathParticle* deathParticle : deathParticles_) {
			if (deathParticle) {
				deathParticle->Draw();
			}
		}
	}
}

TDEngine::Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos = worldTransform_.translation_;
	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Enemy::OnCollision(const Player* player) {

	if (behavior_ == Behavior::kDeath || isHit_) {
		// 敵がやられているなら何もしない
		return;
	}

	// コリジョンを無効化
	isCollisionDisabled_ = true;
	// HPを減らす
	PlayerHitDamage(*player);
}

void Enemy::PlayerHitDamage(const Player& player) {

	if (behavior_ == Behavior::kDeath || behavior_ == Behavior::kChange || behavior_ == Behavior::kStart) {
		// 何もしない
		return;
	}

	if (!isChangeStart_) {

		if (hp_ <= halfHp_) {
			// 敵の振る舞いを変化演出に変更
			isChangeStart_ = true;
			return;
		}
	}

	if (hp_ <= 0) {
		// 敵の振る舞いをデス演出に変更
		behaviorRequest_ = Behavior::kDeath;
		// コリジョン無効
		isCollisionDisabled_ = true;
		return;
	}

	// プレイヤーの大きさに応じてHPを減らす
	hp_ -= player.GetScale().x * 10.0f;
	// ヒットフラグを立てる
	isHit_ = true;
}

void Enemy::BombHitDamage() {
	hp_ -= 10;
	// ヒットフラグを立てる
	isHit_ = true;

	if (!isChangeStart_) {

		if (hp_ <= halfHp_) {
			// 敵の振る舞いを変化演出に変更
			behaviorRequest_ = Behavior::kChange;
			isChangeStart_ = true;
		}
	}

	if (hp_ <= 0) {
		// 敵の振る舞いをデス演出に変更
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
	// ビームの削除
	if (beam_) {
		delete beam_;
	}
	// 針の削除
	for (Needle* needle : needles_) {
		delete needle;
	}
	// 雷の削除
	for (Thunder* thunder : thunders_) {
		delete thunder;
	}

	// パンチの削除
	for (Punch* punch : punches_) {
		delete punch;
	}

	// デス爆発パーティクルの削除
	for (DeathEx* deathEx : deathExs_) {
		delete deathEx;
	}

	// デスパーティクルの削除
	for (EnemyDeathParticle* deathParticle : deathParticles_) {
		delete deathParticle;
	}
}

// 連鎖ボムとの当たり判定
void Enemy::OnCollision(const ChainBomb* chainBomb) {
	// 破壊済みなら何もしない
	if (chainBomb->IsDestroy()) {
		return;
	}

	// ★ 爆発していないボムなら何もしない
	if (!chainBomb->IsExplode()) {
		return;
	}

	BombHitDamage();
}
