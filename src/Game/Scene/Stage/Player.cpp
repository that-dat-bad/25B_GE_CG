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

// KamataEngine名前空間を使用しないため、MyMath::Vector3等はusing前提か省略

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
	// 無敵タイマー
	if (invincibleTimer_ > 0) {
		invincibleTimer_--;
	}

	// 弾・ミサイル削除
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

	// 更新処理
	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}
	for (PlayerMissile* missile : missiles_) {
		missile->Update();
	}

	// --- 自機の移動処理 ---
	Vector3 move = {0, 0, 0};
	const float kCharacterSpeed = 0.2f;

	Vector3 currentRot = object3d_->GetRotate();
	
	float targetRotZ = 0.0f;
	float targetRotX = 0.0f;
	float targetRotY = static_cast<float>(M_PI);
	const float kMaxTilt = 0.5f;
	const float kMaxYaw = 0.3f;

	// 操作許可時のみ入力を受け付ける
	if (isInputEnable) {
		// Aキー (左)
		if (input_->PushKey(DIK_A)) {
			move.x -= kCharacterSpeed;
			targetRotZ = kMaxTilt;
			targetRotY = static_cast<float>(M_PI) - kMaxYaw;
		}
		// Dキー (右)
		else if (input_->PushKey(DIK_D)) {
			move.x += kCharacterSpeed;
			targetRotZ = -kMaxTilt;
			targetRotY = static_cast<float>(M_PI) + kMaxYaw;
		}

		// Wキー (上)
		if (input_->PushKey(DIK_W)) {
			move.y += kCharacterSpeed;
			targetRotX = kMaxTilt;
		}
		// Sキー (下)
		else if (input_->PushKey(DIK_S)) {
			move.y -= kCharacterSpeed;
			targetRotX = -kMaxTilt;
		}

		// 攻撃
		Attack();
	}

	Vector3 pos = object3d_->GetTranslate();
	pos.x += move.x;
	pos.y += move.y;
	pos.z += move.z;

	// 移動制限
	const float kMoveLimitX = 10.0f;
	const float kMoveLimitY = 5.0f;
	pos.x = (std::max)(pos.x, -kMoveLimitX);
	pos.x = (std::min)(pos.x, +kMoveLimitX);
	pos.y = (std::max)(pos.y, -kMoveLimitY);
	pos.y = (std::min)(pos.y, +kMoveLimitY);

	object3d_->SetTranslate(pos);

	// 姿勢制御
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
	// マウス押しっぱなし連射
	if (input_->IsPressMouse(0)) {

		static int frameCount = 0;
		frameCount++;
		if (frameCount % 10 == 0) {
			Vector3 position = object3d_->GetTranslate();
			const float kBulletSpeed = 1.0f;
			Vector3 velocity(0, 0, -kBulletSpeed);
			
			// TransformNormal相当の計算 (回転のみ適用したい)
			// 本来はWorldMatrixから回転だけ取り出して適用するが、ここでは簡易的に現在のY回転等を考慮する？
			// ただ、元のコードは matWorld_ を使っていた。
			// Object3dは行列を持っているはずだが公開されていない場合もある。
			// ここでは弾は常に前(Zマイナス)に飛ぶとして、
			// プレイヤーの姿勢(傾き)に合わせて弾も傾けるかは要検討。
			// 元コード: TransformNormal(velocity, worldTransform_.matWorld_);
			// これは「プレイヤーの向き」に飛ぶということ。
			
			// 簡易実装: プレイヤーの回転行列を再計算して掛けるか、あるいはそのままZ軸マイナスへ飛ばすか。
			// ゲーム性的に「傾きに合わせて弾が斜めに飛ぶ」仕様なら再現が必要。
			// Object3dCommon.h にある TransformNormal が使えるなら使う。
			// 無ければ、自前で計算。
			
			Matrix4x4 matWorld = MakeAffineMatrix({1,1,1}, object3d_->GetRotate(), {0,0,0}); // 回転のみ
			velocity = TransformNormal(velocity, matWorld);

			PlayerBullet* newBullet = new PlayerBullet();
			// Camera* を渡す必要がある
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