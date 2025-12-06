#include "Player.h"
#include "Wind.h"

#include <algorithm>
#include <cmath>
#include <numbers>

using namespace TDEngine;
using namespace MyMath; // TDEngineの数学ライブラリを使用

/// <summary>
/// 初期化
/// </summary>
void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	assert(camera);

	// モデルを受け取る
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.scale = { 1.0f, 1.0f, 1.0f }; // scale_ -> scale
	worldTransform_.translation = position; // translation_ -> translation

	ApplyScaleFromHeight();

	state_ = State::kAlive;

	// デスパーティクルのモデル生成
	modelParticle_ = Model::CreateFromOBJ("playerDeathParticle", true);
	modelFireParticle_ = Model::CreateFromOBJ("firework", true);

	rand_ = new Rand();
	rand_->Initialize();
	rand_->RandomInitialize();

	// ワールドトランスフォーム更新 (UpdateWorldMatrix -> UpdateMatrix)
	worldTransform_.UpdateMatrix();
}
/// <summary>
/// 更新
/// </summary>
void Player::Update() {

	switch (state_) {
	case State::kAlive:

		UpdateAlive();

		break;
	case State::kDead:

		UpdateDead();

		break;
	case State::kRespawn:

		UpdateRespawn();

		break;
	case State::kInvincible:
		UpdateInvincible();
		UpdateAlive();

		break;
	}

	// ワールドトランスフォーム更新
	worldTransform_.UpdateMatrix();
}

/// <summary>
/// 描画
/// </summary>
void Player::Draw() {

	if (state_ == State::kDead) {
		// デスパーティクルの描画
		deathParticle_->Draw();

		for (FireworkParticle* fireworkParticle : fireworkParticles_)
		{
			fireworkParticle->Draw();
		}


	}
	else {
		if (state_ == State::kInvincible) {
			if (frameCount_ % 4 == 0) {
				model_->Draw(worldTransform_, *camera_);
			}
		}
		else {
			model_->Draw(worldTransform_, *camera_);
		}
	}
}

/// <summary>
/// 移動入力処理
/// </summary>
void Player::UpdateMove() {
	// 加速度
	acceleration_ = { 0, 0, 0 };

	// Input::GetInstance() -> GetInput()
	if (GetInput()->PushKey(DIK_D) || GetInput()->PushKey(DIK_A)) {
		if (GetInput()->PushKey(DIK_D)) {
			if (velocity_.x < 0.0f) {
				// 速度と逆方向に入力中はブレーキ
				velocity_.x *= (1.0f - kAttenuation);
			}

			if (state_ == State::kInvincible) {
				acceleration_.x += kAccelerationInvincible;
			}
			else {
				acceleration_.x += kAcceleration;
			}

			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;

				// 旋回開始時の角度を保存 (rotation_ -> rotation)
				turnFirstRotationY_ = worldTransform_.rotation.y;
				// 旋回タイマーをセット
				turnTimer_ = kTimeTurn;
			}
		}
		else if (GetInput()->PushKey(DIK_A)) {
			if (velocity_.x > 0.0f) {
				// 速度と逆方向に入力中はブレーキ
				velocity_.x *= (1.0f - kAttenuation);
			}

			if (state_ == State::kInvincible) {
				acceleration_.x -= kAccelerationInvincible;
			}
			else {
				acceleration_.x -= kAcceleration;
			}

			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;

				// 旋回開始時の角度を保存
				turnFirstRotationY_ = worldTransform_.rotation.y;
				// 旋回タイマーをセット
				turnTimer_ = kTimeTurn;
			}
		}
	}
	else {
		// 入力していないときは移動減衰をかける
		velocity_.x *= (1.0f - kAttenuation);
	}

	if (turnTimer_ > 0.0f) {
		turnTimer_ -= kDeltaTime;

		// イージング
		float turnProgress = std::clamp(1.0f - turnTimer_ / kTimeTurn, 0.0f, 1.0f);
		float smoothTurnProgress = EaseInOutSine(turnProgress);

		// 左右のプレイヤー角度テーブル（ちょっと雑かも）
		float destinationRotationYTable[] = { 0.0f, std::numbers::pi_v<float> *3.0f / 4.0f };
		// 状態に応じた角度を取得
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// プレイヤーの角度を設定 (rotation_ -> rotation)
		worldTransform_.rotation.y = std::lerp(turnFirstRotationY_, destinationRotationY, smoothTurnProgress);
	}

	if (GetInput()->PushKey(DIK_W) || GetInput()->PushKey(DIK_S)) {
		if (GetInput()->PushKey(DIK_W)) {
			if (velocity_.y < 0.0f) {
				// 速度と逆方向に入力中はブレーキ
				velocity_.y *= (1.0f - kAttenuation);
			}

			if (state_ == State::kInvincible) {
				acceleration_.y += kAccelerationInvincible;
			}
			else {
				acceleration_.y += kAcceleration;
			}

		}
		else if (GetInput()->PushKey(DIK_S)) {
			if (velocity_.y > 0.0f) {
				// 速度と逆方向に入力中はブレーキ
				velocity_.y *= (1.0f - kAttenuation);
			}

			if (state_ == State::kInvincible) {
				acceleration_.y -= kAccelerationInvincible;
			}
			else {
				acceleration_.y -= kAcceleration;
			}
		}
	}
	else {
		// 入力していないときは移動減衰をかける
		velocity_.y *= (1.0f - kAttenuation);
	}

	// 加速度を速度に加算
	velocity_ += acceleration_;

	// ある瞬間のフレームの最大速度
	float maxSpeed = kMaxSpeed;

	// 風の影響を受けているときだけ、size_でmaxSpeedを変える
	if (isBlow_) {
		const float borderSize = 5.0f; // size_のデフォルト値6.0f(現状)

		if (size_ < borderSize) {
			// 小さいときは2倍
			maxSpeed = kMaxSpeed * 2.0f;
		}
		else if (size_ > borderSize) {
			// 大きいときは1.2倍
			maxSpeed = kMaxSpeed * 1.2f;
		}
		else {
			maxSpeed = kMaxSpeed;
		}
	}

	// 斜め移動の速度を左右上下移動時と等しくする
	{
		float speed = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
		if (speed > maxSpeed) {
			float scale = maxSpeed / speed;
			velocity_.x *= scale;
			velocity_.y *= scale;
		}
	}

	// 最大速度を制限
	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
	velocity_.y = std::clamp(velocity_.y, -maxSpeed, maxSpeed);
}

// 生存時の更新
void Player::UpdateAlive() {

	// 移動入力処理
	UpdateMove();

	// Input::GetInstance() -> GetInput()
	if (GetInput()->TriggerKey(DIK_SPACE)) {
		if (state_ != State::kInvincible) {
			isExplode_ = true;
			state_ = State::kDead;

			respawnTimer_ = kRespawnTimeSelf;
			// デスパーティクルの初期化 (translation_ -> translation)
			deathParticle_ = new EnemyDeathParticle();
			deathParticle_->Initialize(modelParticle_, camera_, worldTransform_.translation);
			// 爆発パーティクルを生成
			for (int32_t i = 0; i < kFireParticleCount; ++i) {
				FireworkParticle* fireworkParticle = new FireworkParticle();
				// 爆発パーティクルの初期位置設定
				Vector3 pos = worldTransform_.translation;
				pos.x += static_cast<float>(rand_->GetRandom()) - 2.0f;
				pos.y -= static_cast<float>(rand_->GetRandom()) - 2.0f;
				pos.z -= 5.0f;
				fireworkParticle->Initialize(modelFireParticle_, camera_, pos);
				// パーティクルを登録する
				fireworkParticles_.push_back(fireworkParticle);

			}
		}
	}

	// 速度反映
	worldTransform_.translation += velocity_;

	// 画面内に収める
	Vector3& pos = worldTransform_.translation;
	const float radius = 1.0f;
	pos.x = std::clamp(pos.x, kScreenLeft + radius, kScreenRight - radius);
	pos.y = std::clamp(pos.y, kScreenBottom + radius, kScreenTop - radius);

	// 高ければ高いほどプレイヤーが大きくなる
	// bottom～top を 0.0～1.0 に正規化
	float t = (pos.y - kScreenBottom) / (kScreenTop - kScreenBottom);
	t = std::clamp(t, 0.0f, 1.0f);

	// t=0 のとき kMinScale、t=1 のとき kMaxScale
	float scale = kMinScale + t * (kMaxScale - kMinScale);

	// 等方にスケーリング (scale_ -> scale)
	worldTransform_.scale.x = scale;
	worldTransform_.scale.y = scale;
	worldTransform_.scale.z = scale;

	// 当たり判定用
	size_ = 2.0f * scale;

	// 爆発威力
	explosivePower_ = size_;
}
// 死亡時の更新
void Player::UpdateDead() {

	respawnTimer_ -= kDeltaTime;

	if (respawnTimer_ <= 1.0f) {
		StartRespawn();
	}

	// デスパーティクルの更新
	deathParticle_->Update();
	for (FireworkParticle* fireworkParticle : fireworkParticles_) {
		fireworkParticle->Update();
	}
}
// リスポーン時の更新
void Player::UpdateRespawn() {
	// 残り時間を減らす（秒）
	respawnTimer_ -= kDeltaTime;

	// respawnTimer_ は「残り秒数」なので、
	// t = 経過割合 = 1 - (残り / 全体)
	const float easeDuration = 1.0f; // 「1秒かけて戻る」演出
	float t = 1.0f - (respawnTimer_ / easeDuration);
	t = std::clamp(t, 0.0f, 1.0f);

	// イージングで位置を補間
	// worldTransform_.EaseOutFloat -> std::lerp (またはMyMath::Lerp) で代用
	// EaseOut効果をつけたければ t にイージング関数をかける
	float easedT = 1.0f - std::pow(1.0f - t, 3.0f); // EaseOutCubic的な計算

	worldTransform_.translation.x = std::lerp(startPos_.x, endPos_.x, easedT);
	worldTransform_.translation.y = std::lerp(startPos_.y, endPos_.y, easedT);

	// 高さによって大きさが変わる
	ApplyScaleFromHeight();

	if (respawnTimer_ <= 0.0f) {
		state_ = State::kInvincible;

		respawnTimer_ = 0.0f;
		frameCount_ = kFrameCount;
		invincibleTimer_ = kInvincibleTime;
		// デスパーティクルを削除
		for (FireworkParticle* fireworkParticle : fireworkParticles_) {
			delete fireworkParticle;
		}
		fireworkParticles_.clear();
	}
}

// リスポーンスタート
void Player::StartRespawn() {
	state_ = State::kRespawn;

	isAlive_ = true;
	isExplode_ = false;

	// 画面外の開始位置（X=画面左より外、Y=目標と同じ高さ）
	startPos_ = { kScreenLeft - 20.0f, -15.0f, 0.0f };
	endPos_ = { -27.0f, 0.0f, 0.0f };

	// 開始位置を設定
	worldTransform_.translation = startPos_;
	velocity_ = { 0.0f, 0.0f, 0.0f };

	// 高さによって大きさが変わる
	ApplyScaleFromHeight();
}

// 無敵中の更新
void Player::UpdateInvincible() {

	invincibleTimer_ -= kDeltaTime;

	frameCount_--;

	if (invincibleTimer_ <= 0) {
		state_ = State::kAlive;
		frameCount_ = 0;
	}
}

// 高さによってプレイヤーのサイズが変化
void Player::ApplyScaleFromHeight() {
	Vector3& pos = worldTransform_.translation;

	// 1. Yだけ画面内にクランプ（演出でも通常でも使える）
	const float radius = 1.0f;
	pos.y = std::clamp(pos.y, kScreenBottom + radius, kScreenTop - radius);

	// 2. bottom～top を 0.0～1.0 に正規化
	float t = (pos.y - kScreenBottom) / (kScreenTop - kScreenBottom);
	t = std::clamp(t, 0.0f, 1.0f);

	// 3. t=0 → kMinScale,  t=1 → kMaxScale になる scale を計算
	float s = kMinScale + t * (kMaxScale - kMinScale);

	// 4. 等方スケールに反映
	worldTransform_.scale.x = s;
	worldTransform_.scale.y = s;
	worldTransform_.scale.z = s;

	// 5. サイズと爆発威力も決定
	size_ = 2.0f * s;
	explosivePower_ = size_;
}

Vector3 Player::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	// matWorld_ -> matWorld
	worldPos.x = worldTransform_.matWorld.m[3][0];
	worldPos.y = worldTransform_.matWorld.m[3][1];
	worldPos.z = worldTransform_.matWorld.m[3][2];

	return worldPos;
}
AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - size_ / 2.0f, worldPos.y - size_ / 2.0f, worldPos.z - size_ / 2.0f };
	aabb.max = { worldPos.x + size_ / 2.0f, worldPos.y + size_ / 2.0f, worldPos.z + size_ / 2.0f };

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {

	if (!isAlive_) {
		// 敵がやられているなら何もしない
		return;
	}

	if (state_ == State::kInvincible) {
		return;
	}

	(void)enemy;
	isAlive_ = false;
	respawnTimer_ = kRespawnTimeKilled;
	state_ = State::kDead;

	// デスパーティクルの初期化
	deathParticle_ = new EnemyDeathParticle();
	deathParticle_->Initialize(modelParticle_, camera_, worldTransform_.translation);
	// 爆発パーティクルを生成
	for (int32_t i = 0; i < kFireParticleCount; ++i) {
		FireworkParticle* fireworkParticle = new FireworkParticle();
		// 爆発パーティクルの初期位置設定
		Vector3 pos = worldTransform_.translation;
		pos.x += static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.y -= static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.z -= 5.0f;
		fireworkParticle->Initialize(modelFireParticle_, camera_, pos);
		// パーティクルを登録する
		fireworkParticles_.push_back(fireworkParticle);
	}

}

void Player::OnCollision(const Beam* beam) {
	if (!isAlive_) {
		// 敵がやられているなら何もしない
		return;
	}

	if (state_ == State::kInvincible || state_ == State::kRespawn) {
		return;
	}

	(void)beam;
	isAlive_ = false;

	respawnTimer_ = kRespawnTimeKilled;
	state_ = State::kDead;
	// デスパーティクルの初期化
	deathParticle_ = new EnemyDeathParticle();
	deathParticle_->Initialize(modelParticle_, camera_, worldTransform_.translation);
	// 爆発パーティクルを生成
	for (int32_t i = 0; i < kFireParticleCount; ++i) {
		FireworkParticle* fireworkParticle = new FireworkParticle();
		// 爆発パーティクルの初期位置設定
		Vector3 pos = worldTransform_.translation;
		pos.x += static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.y -= static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.z -= 5.0f;
		fireworkParticle->Initialize(modelFireParticle_, camera_, pos);
		// パーティクルを登録する
		fireworkParticles_.push_back(fireworkParticle);
	}
}

// 風との衝突応答
void Player::OnCollision(const Wind* wind) {
	(void)wind;
	isBlow_ = true;

	velocity_ += wind->GetVelocity() / (powf(size_, 3.0f) / 3.0f);
}

void Player::OnCollision(const Needle* needle) {
	if (!isAlive_) {
		// 敵がやられているなら何もしない
		return;
	}

	if (state_ == State::kInvincible || state_ == State::kRespawn) {
		return;
	}

	(void)needle;
	isAlive_ = false;

	respawnTimer_ = kRespawnTimeKilled;
	state_ = State::kDead;
	// デスパーティクルの初期化
	deathParticle_ = new EnemyDeathParticle();
	deathParticle_->Initialize(modelParticle_, camera_, worldTransform_.translation);
	// 爆発パーティクルを生成
	for (int32_t i = 0; i < kFireParticleCount; ++i) {
		FireworkParticle* fireworkParticle = new FireworkParticle();
		// 爆発パーティクルの初期位置設定
		Vector3 pos = worldTransform_.translation;
		pos.x += static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.y -= static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.z -= 5.0f;
		fireworkParticle->Initialize(modelFireParticle_, camera_, pos);
		// パーティクルを登録する
		fireworkParticles_.push_back(fireworkParticle);
	}
}

void Player::OnCollision(const Punch* punch) {
	if (!isAlive_) {
		// 敵がやられているなら何もしない
		return;
	}

	if (state_ == State::kInvincible || state_ == State::kRespawn) {
		return;
	}

	(void)punch;
	isAlive_ = false;

	respawnTimer_ = kRespawnTimeKilled;
	state_ = State::kDead;
	// デスパーティクルの初期化
	deathParticle_ = new EnemyDeathParticle();
	deathParticle_->Initialize(modelParticle_, camera_, worldTransform_.translation);
	// 爆発パーティクルを生成
	for (int32_t i = 0; i < kFireParticleCount; ++i) {
		FireworkParticle* fireworkParticle = new FireworkParticle();
		// 爆発パーティクルの初期位置設定
		Vector3 pos = worldTransform_.translation;
		pos.x += static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.y -= static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.z -= 5.0f;
		fireworkParticle->Initialize(modelFireParticle_, camera_, pos);
		// パーティクルを登録する
		fireworkParticles_.push_back(fireworkParticle);
	}
}

void Player::OnCollision(const Thunder* thunder) {
	if (!isAlive_) {
		// 敵がやられているなら何もしない
		return;
	}

	if (state_ == State::kInvincible || state_ == State::kRespawn) {
		return;
	}

	(void)thunder;
	isAlive_ = false;

	respawnTimer_ = kRespawnTimeKilled;
	state_ = State::kDead;
	// デスパーティクルの初期化
	deathParticle_ = new EnemyDeathParticle();
	deathParticle_->Initialize(modelParticle_, camera_, worldTransform_.translation);
	// 爆発パーティクルを生成
	for (int32_t i = 0; i < kFireParticleCount; ++i) {
		FireworkParticle* fireworkParticle = new FireworkParticle();
		// 爆発パーティクルの初期位置設定
		Vector3 pos = worldTransform_.translation;
		pos.x += static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.y -= static_cast<float>(rand_->GetRandom()) - 2.0f;
		pos.z -= 5.0f;
		fireworkParticle->Initialize(modelFireParticle_, camera_, pos);
		// パーティクルを登録する
		fireworkParticles_.push_back(fireworkParticle);
	}
}

/// <summary>
/// イージング
/// </summary>
float Player::EaseInOutSine(float turnProgress) { return -(std::cos(std::numbers::pi_v<float> *turnProgress) - 1.0f) * 0.5f; }