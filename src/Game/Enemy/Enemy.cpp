#include "Enemy.h"
#include "../../engine/Graphics/Model/Object3d.h"
#include "../../engine/Graphics/Model/Object3dCommon.h"
#include "../../engine/Graphics/Camera/CameraManager.h"
#include "../../engine/Graphics/Model/ModelManager.h"
#include "../../engine/Graphics/Particle/EffectManager.h"
#include "../../engine/Graphics/Particle/ParticleManager.h"

void Enemy::Initialize(
	const MyMath::Vector3& position,
	const std::string& modelPath,
	float maxHealth,
	const AirframeData& airframeData,
	const EngineData& engineData,
	const GunPodData& gunpodData,
	FlightModel* playerFlightModel,
	BulletManager* bulletManager
) {
	maxHealth_ = maxHealth;
	health_ = maxHealth;
	isAlive_ = true;
	playerFlightModel_ = playerFlightModel;
	bulletManager_ = bulletManager;

	// フライトモデルと武装の初期化
	flightModel_.Initialize(airframeData, engineData);
	flightModel_.SetPosition(position);
	flightModel_.SetVelocity(MyMath::Multiply(200.0f / 3.6f, flightModel_.GetForwardDirection())); // 初期速度
	flightModel_.SetThrottle(0.8f);

	gunpod_.Initialize(gunpodData);
	
	aimController_.Initialize();
	aimController_.SetEnabled(true);

	// モデルの読み込み
	ModelManager::GetInstance()->LoadModel(modelPath);

	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	object3d_->SetModel(modelPath);

	// 見た目の設定 (敵機用により光沢を下げるなど適宜変更可能だが、今回は同じモデルを使用)
	object3d_->GetModel()->SetEnvironmentCoefficient(0.4f);
	object3d_->GetModel()->SetSpecularIntensity(1.5f);

	// スケール
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });
}

// クォータニオンからオイラー角への変換ヘルパー
static MyMath::Vector3 QuaternionToEuler(const MyMath::Quaternion& q) {
	MyMath::Vector3 euler{};

	// ロール (X軸回り)
	float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	euler.x = std::atan2(sinr_cosp, cosr_cosp);

	// ピッチ (Y軸回り)
	float sinp = 2.0f * (q.w * q.y - q.z * q.x);
	if (std::fabs(sinp) >= 1.0f) {
		euler.y = std::copysign(3.14159265f / 2.0f, sinp);
	} else {
		euler.y = std::asin(sinp);
	}

	// ヨー (Z軸回り)
	float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	euler.z = std::atan2(siny_cosp, cosy_cosp);

	return euler;
}

void Enemy::UpdateAI(float deltaTime) {
	if (!playerFlightModel_) return;

	MyMath::Vector3 playerPos = playerFlightModel_->GetPosition();
	MyMath::Vector3 myPos = flightModel_.GetPosition();
	
	MyMath::Vector3 toPlayer = MyMath::Substract(playerPos, myPos);
	float distance = MyMath::Length(toPlayer);

	// 偏差射撃（リード）の計算：弾速と相手の速度から未来位置を予測する
	float bulletSpeed = flightModel_.GetSpeed() + 500.0f; // 弾速
	MyMath::Vector3 targetVel = playerFlightModel_->GetVelocity();
	
	// 1次予測
	float timeToHit = distance / bulletSpeed;
	MyMath::Vector3 futurePos = MyMath::Add(playerPos, MyMath::Multiply(timeToHit, targetVel));
	
	// 2次予測（より正確な着弾点）
	float distance2 = MyMath::Length(MyMath::Substract(futurePos, myPos));
	timeToHit = distance2 / bulletSpeed;
	futurePos = MyMath::Add(playerPos, MyMath::Multiply(timeToHit, targetVel));

	MyMath::Vector3 toFuture = MyMath::Substract(futurePos, myPos);

	// 目標方向を未来位置へセット
	if (distance > 1.0f) {
		MyMath::Vector3 targetDir = MyMath::Normalize(toFuture);
		aimController_.ResetToDirection(targetDir);
	}

	// 操舵入力の計算
	float finalPitch = 0.0f, finalRoll = 0.0f, finalYaw = 0.0f;
	aimController_.CalculateSteeringInput(flightModel_.GetOrientation(), deltaTime, finalPitch, finalRoll, finalYaw);
	flightModel_.SetControlInput(finalPitch, finalRoll, finalYaw);

	// スロットル調整 (遠ければ加速、近ければ減速)
	if (distance > 1000.0f) {
		flightModel_.SetThrottleInput(1.0f);
	} else if (distance < 300.0f) {
		flightModel_.SetThrottleInput(0.4f);
	} else {
		flightModel_.SetThrottleInput(0.7f);
	}

	// 攻撃判定
	MyMath::Vector3 forward = flightModel_.GetForwardDirection();
	// 予測位置(toFuture)に対して正面を向いているかで射撃判定を行う
	float dot = MyMath::Dot(forward, MyMath::Normalize(toFuture));
	// プレイヤー（の未来位置）が正面(約5度以内)におり、かつ距離が1000m以下なら射撃
	if (dot > 0.996f && distance < 1000.0f) {
		int ammoBefore = gunpod_.GetCurrentAmmo();
		gunpod_.Fire();
		
		if (gunpod_.GetCurrentAmmo() < ammoBefore) { // 発射できた場合
			MyMath::Vector3 firePos = flightModel_.GetPosition();
			MyMath::Vector3 aircraftVelocity = flightModel_.GetVelocity();
			
			// 機首から発射
			firePos = MyMath::Add(firePos, MyMath::Multiply(8.0f, forward));
			
			// 弾丸初速ベクトル ＝ 機体速度 ＋ (射出方向 × 射出速度)
			float muzzleSpeed = 500.0f;
			MyMath::Vector3 muzzleVelocity = MyMath::Multiply(muzzleSpeed, forward);
			MyMath::Vector3 bulletVelocity = MyMath::Add(aircraftVelocity, muzzleVelocity);

			if (bulletManager_) {
				bulletManager_->SpawnBullet(firePos, bulletVelocity, 10.0f, true);
			}

			// マズルフラッシュ
			muzzleFlashCount_++;
			if (muzzleFlashCount_ % 2 == 0) {
				EffectManager::GetInstance()->EmitMuzzleFlash(firePos, forward);
				muzzleFlashTimer_ = 0.08f;
			}
		}
	}
}

void Enemy::Update(float deltaTime) {
	if (!isAlive_) { return; }

	UpdateAI(deltaTime);
	flightModel_.Update(deltaTime);

	if (muzzleFlashTimer_ > 0.0f) {
		muzzleFlashTimer_ -= deltaTime;
	}

	// オブジェクトに位置と姿勢を適用
	object3d_->SetTranslate(flightModel_.GetPosition());
	object3d_->SetRotate(QuaternionToEuler(flightModel_.GetOrientation()));
	object3d_->Update();
}

void Enemy::Draw() {
	if (!isAlive_) { return; }
	if (!object3d_) { return; }

	object3d_->Draw();
}

void Enemy::TakeDamage(float damage) {
	if (!isAlive_) { return; }

	health_ -= damage;
	if (health_ <= 0.0f) {
		health_ = 0.0f;
		isAlive_ = false;
	}
}

void Enemy::OnCollision(ICollisionBody3D* other) {
	if (!isAlive_) { return; }

	// PlayerBullet との衝突 → ダメージを受ける
	if (other->GetCollisionAttribute() & CollisionAttribute::kPlayerBullet) {
		TakeDamage(10.0f);
		EffectManager::GetInstance()->EmitHitEffect(flightModel_.GetPosition());

		if (!isAlive_) {
			EffectManager::GetInstance()->EmitDestroyEffect(flightModel_.GetPosition());
		}
	}
}
