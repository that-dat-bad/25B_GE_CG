#include "Player.h"
#include "TDEngine.h"
#include "Wind.h"
#include "Enemy.h"


#include "EnemyDeathParticle.h" 
#include "FireworkParticle.h"
#include "Rand.h"

#include <algorithm>
#include <cmath>
#include <numbers>

using namespace MyMath;
using namespace TDEngine;

void Player::Initialize(const Vector3& position) {
	std::string path = "./Resources/Player/Player.obj";
	Model::LoadFromOBJ(path);
	// Object3d生成
	object3d_ = Object3d::Create();
	object3d_->SetModel(path);

	// 初期座標設定
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });

	ApplyScaleFromHeight();

	state_ = State::kAlive;

	// Rand初期化
	rand_ = new Rand();
	rand_->Initialize();
	rand_->RandomInitialize();
}

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

	// TDEngineのObject3dは自身のUpdateで行列更新を行う
	if (object3d_) {
		object3d_->Update();
	}
}

void Player::Draw() {
	if (state_ == State::kDead) {
		// デスパーティクルの描画
		if (deathParticle_) deathParticle_->Draw();
		for (auto& particle : fireworkParticles_) {
			particle->Draw();
		}
	}
	else {
		if (state_ == State::kInvincible) {
			if (frameCount_ % 4 == 0) {
				object3d_->Draw();
			}
		}
		else {
			object3d_->Draw();
		}
	}
}

void Player::UpdateMove() {
	// 加速度リセット
	acceleration_ = { 0, 0, 0 };
	Input* input = TDEngine::GetInput();

	// 左右移動
	if (input->pushKey(DIK_D) || input->pushKey(DIK_A)) {
		if (input->pushKey(DIK_D)) {
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration_.x += (state_ == State::kInvincible) ? kAccelerationInvincible : kAcceleration;

			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = object3d_->GetRotate().y;
				turnTimer_ = kTimeTurn;
			}
		}
		else if (input->pushKey(DIK_A)) {
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration_.x -= (state_ == State::kInvincible) ? kAccelerationInvincible : kAcceleration;

			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = object3d_->GetRotate().y;
				turnTimer_ = kTimeTurn;
			}
		}
	}
	else {
		velocity_.x *= (1.0f - kAttenuation);
	}

	// 旋回処理
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= kDeltaTime;
		float turnProgress = std::clamp(1.0f - turnTimer_ / kTimeTurn, 0.0f, 1.0f);
		float smoothTurnProgress = MyMath::EaseInOutSine(turnProgress);

		float destinationRotationYTable[] = { 0.0f, 3.14159265f * 3.0f / 4.0f };
		float destRotY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		Vector3 currentRot = object3d_->GetRotate();
		currentRot.y = std::lerp(turnFirstRotationY_, destRotY, smoothTurnProgress);
		object3d_->SetRotate(currentRot);
	}

	// 上下移動
	if (input->pushKey(DIK_W) || input->pushKey(DIK_S)) {
		if (input->pushKey(DIK_W)) {
			if (velocity_.y < 0.0f) velocity_.y *= (1.0f - kAttenuation);
			acceleration_.y += (state_ == State::kInvincible) ? kAccelerationInvincible : kAcceleration;
		}
		else if (input->pushKey(DIK_S)) {
			if (velocity_.y > 0.0f) velocity_.y *= (1.0f - kAttenuation);
			acceleration_.y -= (state_ == State::kInvincible) ? kAccelerationInvincible : kAcceleration;
		}
	}
	else {
		velocity_.y *= (1.0f - kAttenuation);
	}

	// 加速度を速度に加算
	velocity_ = Add(velocity_, acceleration_);

	float maxSpeed = kMaxSpeed;
	if (isBlow_) {
		const float borderSize = 5.0f;
		if (size_ < borderSize) maxSpeed = kMaxSpeed * 2.0f;
		else if (size_ > borderSize) maxSpeed = kMaxSpeed * 1.2f;
	}

	// 速度制限
	float speed = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
	if (speed > maxSpeed) {
		float scale = maxSpeed / speed;
		velocity_ = Multiply(scale, velocity_);
	}
	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
	velocity_.y = std::clamp(velocity_.y, -maxSpeed, maxSpeed);
}

void Player::UpdateAlive() {
	UpdateMove();

	if (TDEngine::GetInput()->triggerKey(DIK_SPACE)) {
		if (state_ != State::kInvincible) {
			isExplode_ = true;
			state_ = State::kDead;
			respawnTimer_ = kRespawnTimeSelf;

			// パーティクル生成 (TDEngine対応版のコンストラクタを使う想定)
			deathParticle_ = new EnemyDeathParticle();
			deathParticle_->Initialize(object3d_->GetTranslate());

			for (int32_t i = 0; i < kFireParticleCount; ++i) {
				FireworkParticle* firework = new FireworkParticle();
				Vector3 pos = object3d_->GetTranslate();
				pos.x += static_cast<float>(rand_->GetRandom()) - 2.0f;
				pos.y -= static_cast<float>(rand_->GetRandom()) - 2.0f;
				pos.z -= 5.0f;
				firework->Initialize(pos);
				fireworkParticles_.push_back(firework);
			}
		}
	}

	// 移動反映
	Vector3 currentPos = object3d_->GetTranslate();
	currentPos = Add(currentPos, velocity_);

	// 画面内クランプ
	const float radius = 1.0f;
	currentPos.x = std::clamp(currentPos.x, kScreenLeft + radius, kScreenRight - radius);
	currentPos.y = std::clamp(currentPos.y, kScreenBottom + radius, kScreenTop - radius);
	object3d_->SetTranslate(currentPos);

	ApplyScaleFromHeight();
}

void Player::UpdateDead() {
	respawnTimer_ -= kDeltaTime;
	if (respawnTimer_ <= 1.0f) {
		StartRespawn();
	}

	if (deathParticle_) deathParticle_->Update();
	for (auto& p : fireworkParticles_) p->Update();
}

void Player::UpdateRespawn() {
	respawnTimer_ -= kDeltaTime;
	const float easeDuration = 1.0f;
	float t = 1.0f - (respawnTimer_ / easeDuration);
	t = std::clamp(t, 0.0f, 1.0f);

	Vector3 newPos = object3d_->GetTranslate();
	newPos.x = MyMath::EaseOut(t, startPos_.x, endPos_.x);
	newPos.y = MyMath::EaseOut(t, startPos_.y, endPos_.y);
	object3d_->SetTranslate(newPos);

	ApplyScaleFromHeight();

	if (respawnTimer_ <= 0.0f) {
		state_ = State::kInvincible;
		respawnTimer_ = 0.0f;
		frameCount_ = kFrameCount;
		invincibleTimer_ = kInvincibleTime;

		for (auto& p : fireworkParticles_) delete p;
		fireworkParticles_.clear();
	}
}

void Player::StartRespawn() {
	state_ = State::kRespawn;
	isAlive_ = true;
	isExplode_ = false;

	startPos_ = { kScreenLeft - 20.0f, -15.0f, 0.0f };
	endPos_ = { -27.0f, 0.0f, 0.0f };

	object3d_->SetTranslate(startPos_);
	velocity_ = { 0.0f, 0.0f, 0.0f };
	ApplyScaleFromHeight();
}

void Player::UpdateInvincible() {
	invincibleTimer_ -= kDeltaTime;
	frameCount_--;
	if (invincibleTimer_ <= 0) {
		state_ = State::kAlive;
		frameCount_ = 0;
	}
}

void Player::ApplyScaleFromHeight() {
	Vector3 pos = object3d_->GetTranslate();
	const float radius = 1.0f;
	// 念のためクランプ（計算用）
	float clampedY = std::clamp(pos.y, kScreenBottom + radius, kScreenTop - radius);

	float t = (clampedY - kScreenBottom) / (kScreenTop - kScreenBottom);
	t = std::clamp(t, 0.0f, 1.0f);

	float s = kMinScale + t * (kMaxScale - kMinScale);

	object3d_->SetScale({ s, s, s });
	size_ = 2.0f * s;
	explosivePower_ = size_;
}

Vector3 Player::GetWorldPosition() const {
	// Object3dがTransformを持っているのでそこから取得
	return object3d_->GetTranslate();
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - size_ / 2.0f, worldPos.y - size_ / 2.0f, worldPos.z - size_ / 2.0f };
	aabb.max = { worldPos.x + size_ / 2.0f, worldPos.y + size_ / 2.0f, worldPos.z + size_ / 2.0f };
	return aabb;
}

Vector3 Player::GetScale() const {
	return object3d_->GetScale();
}

void Player::SetPosition(const Vector3& position) {
	object3d_->SetTranslate(position);
}

// 衝突応答 (死亡処理共通)
void Player::OnCollision(const Enemy* enemy) {
	if (!isAlive_ || state_ == State::kInvincible) return;

	isAlive_ = false;
	respawnTimer_ = kRespawnTimeKilled;
	state_ = State::kDead;

	// パーティクル生成 (簡易化)
	if (deathParticle_) delete deathParticle_;
	deathParticle_ = new EnemyDeathParticle();
	deathParticle_->Initialize(object3d_->GetTranslate());
}

// 他のOnCollisionも同様に処理（引数違いなだけなので省略可能ですが、実装としては同様です）
void Player::OnCollision(const Beam* beam) { OnCollision((const Enemy*)nullptr); }
void Player::OnCollision(const Needle* needle) { OnCollision((const Enemy*)nullptr); }
void Player::OnCollision(const Punch* punch) { OnCollision((const Enemy*)nullptr); }
void Player::OnCollision(const Thunder* thunder) { OnCollision((const Enemy*)nullptr); }

void Player::OnCollision(const Wind* wind) {
	isBlow_ = true;
	// 風の影響計算
	Vector3 windVel = wind->GetVelocity();
	float factor = (powf(size_, 3.0f) / 3.0f);
	Vector3 addVel = { windVel.x / factor, windVel.y / factor, windVel.z / factor };
	velocity_ = Add(velocity_, addVel);
}