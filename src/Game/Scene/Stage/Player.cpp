#include "Player.h"
#include "Enemy.h"

#include <DirectXMath.h>
#include <algorithm>
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Object3dCommon.h"
#include "Input.h"
#include "PlayerBullet.h"
#include "PlayerMissile.h"


Player::~Player() {
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	for (PlayerMissile* missile : missiles_) {
		delete missile;
	}
}

void Player::Initialize(Model* model, Camera* camera) {
	assert(model);
	
	camera_ = camera;

	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(model);
	object3d_->SetCamera(camera);
	
	object3d_->SetRotate({ 0.0f, static_cast<float>(M_PI), 0.0f });

	input_ = Input::GetInstance();

	hp_ = kMaxHP_;
	lives_ = kDefaultLives_;
	isDead_ = false;
	invincibleTimer_ = 0;
}

void Player::Update(bool isInputEnable) {
	if (invincibleTimer_ > 0) {
		invincibleTimer_--;
	}

	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});
	missiles_.remove_if([](PlayerMissile* missile) {
		if (missile->IsDead()) {
			delete missile;
			return true;
		}
		return false;
	});

	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}
	for (PlayerMissile* missile : missiles_) {
		missile->Update();
	}

	Vector3 move = {0, 0, 0};
	const float kCharacterSpeed = 0.2f;

	Vector3 currentRot = object3d_->GetRotate();
	
	float targetRotZ = 0.0f;
	float targetRotX = 0.0f;
	float targetRotY = static_cast<float>(M_PI);
	const float kMaxTilt = 0.5f;
	const float kMaxYaw = 0.3f;

	if (isInputEnable) {
		if (input_->PushKey(DIK_A)) {
			move.x -= kCharacterSpeed;
			targetRotZ = kMaxTilt;
			targetRotY = static_cast<float>(M_PI) - kMaxYaw;
		}
		else if (input_->PushKey(DIK_D)) {
			move.x += kCharacterSpeed;
			targetRotZ = -kMaxTilt;
			targetRotY = static_cast<float>(M_PI) + kMaxYaw;
		}

		if (input_->PushKey(DIK_W)) {
			move.y += kCharacterSpeed;
			targetRotX = kMaxTilt;
		}
		else if (input_->PushKey(DIK_S)) {
			move.y -= kCharacterSpeed;
			targetRotX = -kMaxTilt;
		}

		Attack();
	}

	Vector3 pos = object3d_->GetTranslate();
	pos.x += move.x;
	pos.y += move.y;
	pos.z += move.z;

	const float kMoveLimitX = 10.0f;
	const float kMoveLimitY = 5.0f;
	pos.x = (std::max)(pos.x, -kMoveLimitX);
	pos.x = (std::min)(pos.x, +kMoveLimitX);
	pos.y = (std::max)(pos.y, -kMoveLimitY);
	pos.y = (std::min)(pos.y, +kMoveLimitY);

	object3d_->SetTranslate(pos);

	const float kTiltSpeed = 0.1f;
	currentRot.z = LerpShort(currentRot.z, targetRotZ, kTiltSpeed);
	currentRot.x = LerpShort(currentRot.x, targetRotX, kTiltSpeed);
	currentRot.y = LerpShort(currentRot.y, targetRotY, kTiltSpeed);
	
	object3d_->SetRotate(currentRot);

	object3d_->Update();
}

void Player::Draw() {
	if (invincibleTimer_ % 4 < 2) {
		if(object3d_) object3d_->Draw();
	}

	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw();
	}
	for (PlayerMissile* missile : missiles_) {
		missile->Draw();
	}
}

void Player::Attack() {
	if (input_->PushMouse(0)) {

		static int frameCount = 0;
		frameCount++;
		if (frameCount % 10 == 0) {
			Vector3 position = object3d_->GetTranslate();
			const float kBulletSpeed = 1.0f;
			Vector3 velocity(0, 0, -kBulletSpeed);
			
			
			
			Vector3 scale = { 1.0f, 1.0f, 1.0f };
			Vector3 translate = { 0.0f, 0.0f, 0.0f };
			Matrix4x4 matWorld = MakeAffineMatrix(scale, object3d_->GetRotate(), translate); 
			velocity = TransformNormal(velocity, matWorld);

			PlayerBullet* newBullet = new PlayerBullet();
			newBullet->Initialize(bulletModel_, position, velocity, camera_);
			bullets_.push_back(newBullet);
		}
	}
}

void Player::FireMissile(Enemy* target) {
	if (!target || target->IsDead())
		return;
	Vector3 position = object3d_->GetTranslate();
	position.y -= 1.0f;
	PlayerMissile* newMissile = new PlayerMissile();
	newMissile->Initialize(missileModel_, position, target, camera_);
	missiles_.push_back(newMissile);
}

void Player::OnCollision() {
	if (invincibleTimer_ > 0)
		return;

	hp_--;
	invincibleTimer_ = 60;

	if (hp_ <= 0) {
		lives_--;
		if (lives_ > 0) {
			hp_ = kMaxHP_;
			invincibleTimer_ = 120;
		} else {
			isDead_ = true;
		}
	}
}

Vector3 Player::GetWorldPosition() const {
	if (object3d_) return object3d_->GetTranslate();
	return { 0,0,0 };
}

Vector3 Player::GetRotation() const {
	if (object3d_) return object3d_->GetRotate();
	return { 0,0,0 };
}
