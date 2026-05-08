#include "FlightModel.h"
#include <cmath>
#include <algorithm>

// --- 定数 ---
namespace {
	constexpr float kGravity = 9.81f;            // 重力加速度 (m/s^2)
	constexpr float kSeaLevelDensity = 1.225f;   // 海面気圧での空気密度 (kg/m^3)
	constexpr float kScaleHeight = 8500.0f;      // 大気のスケールハイト (m)
	constexpr float kPi = 3.14159265358979f;

	constexpr float kPitchRate = 1.5f;           // ピッチ角速度 (rad/s)
	constexpr float kRollRate = 2.5f;            // ロール角速度 (rad/s)
	constexpr float kYawRate = 0.8f;             // ヨー角速度 (rad/s)

	constexpr float kGDamageRate = 0.05f;        // G超過時の翼ダメージレート
	constexpr float kBlackoutOnsetG = 5.0f;      // ブラックアウト開始G
	constexpr float kRedoutOnsetG = -2.0f;       // レッドアウト開始G
	constexpr float kGEffectBuildRate = 0.1f;     // G効果の増加速度
	constexpr float kGEffectRecoveryRate = 0.5f;  // G効果の回復速度
}


// ===========================================================
// コンストラクタ
// ===========================================================
FlightModel::FlightModel()
	: position_{ 0.0f, 0.0f, 0.0f }
	, velocity_{ 0.0f, 0.0f, 0.0f }
	, acceleration_{ 0.0f, 0.0f, 0.0f }
	, orientation_{ 0.0f, 0.0f, 0.0f, 1.0f } // 単位クォータニオン（無回転）
	, currentAoA_(0.0f)
	, currentG_(1.0f)
	, isStalling_(false)
	, blackoutFactor_(0.0f)
	, redoutFactor_(0.0f)
	, targetThrottle_(0.0f)
	, inputPitch_(0.0f)
	, inputRoll_(0.0f)
	, inputYaw_(0.0f)
{
}


// ===========================================================
// 初期化
// ===========================================================
void FlightModel::Initialize(const AirframeData& airframeData, const EngineData& engineData)
{
	airframe_.Initialize(airframeData);
	engine_.Initialize(engineData);

	// 物理ステータスをリセット
	position_ = { 0.0f, 0.0f, 0.0f };
	velocity_ = { 0.0f, 0.0f, 0.0f };
	acceleration_ = { 0.0f, 0.0f, 0.0f };
	orientation_ = { 0.0f, 0.0f, 0.0f, 1.0f };

	// 拡張状態のリセット
	currentAoA_ = 0.0f;
	currentG_ = 1.0f;
	isStalling_ = false;
	blackoutFactor_ = 0.0f;
	redoutFactor_ = 0.0f;

	// 入力状態をリセット
	targetThrottle_ = 0.0f;
	inputPitch_ = 0.0f;
	inputRoll_ = 0.0f;
	inputYaw_ = 0.0f;
}


// ===========================================================
// 毎フレームの更新処理
// ===========================================================
void FlightModel::Update(float deltaTime)
{
	// 1. エンジンの更新（スロットルのスプールアップ/ダウン）
	engine_.Update(deltaTime, targetThrottle_);

	// 2. 燃料消費 + 燃料漏れ（ダメージ連動）
	float fuelConsumption = engine_.GetFuelConsumptionRate() * deltaTime;
	float fuelLeak = airframe_.GetFuelLeakRate() * deltaTime;
	airframe_.ConsumeInternalFuel(fuelConsumption + fuelLeak);

	// 3. フラップ・エアブレーキの更新
	float speed = MyMath::Length(velocity_);
	airframe_.UpdateFlap(deltaTime, speed);
	airframe_.UpdateAirBrake(deltaTime);

	// 4. ペイロードの更新
	for (auto& payload : payloads_) {
		payload->Update(deltaTime);
	}

	// 5. AoA計算（姿勢更新前に現在のAoAを知っておく）
	currentAoA_ = CalculateAoA();

	// 失速判定
	isStalling_ = std::fabs(currentAoA_) > airframe_.GetCriticalAoA();

	// 6. 姿勢の更新（速度依存 + ダメージ連動の操舵）
	UpdateOrientation(deltaTime);

	// 7. 合力を計算
	MyMath::Vector3 totalForce = CalculateTotalForce(deltaTime);

	// 8. 加速度 = 力 / 質量 (F = ma → a = F/m)
	float totalMass = GetTotalMass();
	MyMath::Vector3 prevVelocity = velocity_;

	if (totalMass > 0.0f) {
		float invMass = 1.0f / totalMass;
		acceleration_ = MyMath::Multiply(invMass, totalForce);
	}

	// 9. 速度の更新 (v += a * dt)
	velocity_ = MyMath::Add(velocity_, MyMath::Multiply(deltaTime, acceleration_));

	// 10. 位置の更新 (p += v * dt)
	position_ = MyMath::Add(position_, MyMath::Multiply(deltaTime, velocity_));

	// 11. G計算
	// G = (速度変化量 / dt) のうち、重力を除いた成分 / 重力加速度
	if (deltaTime > 0.0001f) {
		MyMath::Vector3 deltaV = MyMath::Substract(velocity_, prevVelocity);
		MyMath::Vector3 accelNoGravity = MyMath::Multiply(1.0f / deltaTime, deltaV);
		// 重力を除外（重力は体感Gに含めない: 自由落下 = 0G）
		accelNoGravity = MyMath::Add(accelNoGravity, { 0.0f, kGravity, 0.0f });

		// 機体のUp軸方向のG成分（パイロットが感じるG）
		MyMath::Vector3 up = GetUpDirection();
		currentG_ = MyMath::Dot(accelNoGravity, up) / kGravity;
	}

	// 12. G効果の更新（ブラックアウト/レッドアウト + 構造ダメージ）
	UpdateGEffects(deltaTime);
}


// ===========================================================
// ペイロードの追加
// ===========================================================
void FlightModel::AddPayload(std::unique_ptr<IPayload> payload)
{
	payloads_.push_back(std::move(payload));
}


// ===========================================================
// 全武装を発射
// ===========================================================
void FlightModel::FireWeapons()
{
	for (auto& payload : payloads_) {
		payload->Fire();
	}
}


// ===========================================================
// 総質量の取得（機体 + エンジン + 全ペイロード）
// ===========================================================
float FlightModel::GetTotalMass() const
{
	float mass = airframe_.GetTotalMass() + engine_.GetMass();

	for (const auto& payload : payloads_) {
		mass += payload->GetWeight();
	}

	return mass;
}


// ===========================================================
// ISA標準大気モデル（簡易版）による空気密度計算
// ===========================================================
float FlightModel::CalculateAirDensity(float altitude) const
{
	// 海面: 1.225 kg/m³, 指数的に減少
	// 負の高度（海面以下）はクランプ
	float alt = (std::max)(altitude, 0.0f);
	return kSeaLevelDensity * std::exp(-alt / kScaleHeight);
}


// ===========================================================
// 迎え角（AoA）の計算
// ===========================================================
float FlightModel::CalculateAoA() const
{
	float speed = MyMath::Length(velocity_);
	if (speed < 1.0f) return 0.0f;

	MyMath::Vector3 velDir = MyMath::Normalize(velocity_);
	MyMath::Vector3 forward = GetForwardDirection();

	// forward と velDir の角度差
	float dot = MyMath::Dot(forward, velDir);
	dot = std::clamp(dot, -1.0f, 1.0f);
	float angle = std::acos(dot);

	// 符号判定: 機体の上方向と速度の関係
	MyMath::Vector3 up = GetUpDirection();
	if (MyMath::Dot(up, velDir) > 0.0f) {
		angle = -angle; // 機首下げ方向は負
	}

	return angle;
}


// ===========================================================
// 迎え角に応じた揚力係数の計算
// ===========================================================
float FlightModel::CalculateLiftCoefficient(float aoa) const
{
	float absAoA = std::fabs(aoa);
	float criticalAoA = airframe_.GetCriticalAoA();
	float maxCL = airframe_.GetMaxLiftCoefficient();
	float stallCL = airframe_.GetStallLiftCoefficient();

	float cl;
	if (absAoA <= criticalAoA) {
		// 臨界角以下: AoAに比例して揚力係数が増加
		cl = maxCL * (absAoA / criticalAoA);
	} else {
		// 失速域: 臨界角を超えると急激に低下
		float overStallRatio = (absAoA - criticalAoA) / criticalAoA;
		overStallRatio = (std::min)(overStallRatio, 1.0f);
		cl = maxCL - (maxCL - stallCL) * overStallRatio;
	}

	// AoAの符号に合わせて揚力の方向を決定
	if (aoa < 0.0f) cl = -cl;

	return cl;
}


// ===========================================================
// 速度依存の操舵効率
// War Thunder風: 低速→弱い、中速→最高、高速→重い
// ===========================================================
float FlightModel::CalculateControlEfficiency(float speed) const
{
	if (speed < 30.0f)  return speed / 30.0f * 0.3f;           // 低速域: ほぼ効かない
	if (speed < 100.0f) return 0.3f + (speed - 30.0f) / 70.0f * 0.7f; // 遷移域
	if (speed < 300.0f) return 1.0f;                            // 最適域
	if (speed < 500.0f) return 1.0f - (speed - 300.0f) / 200.0f * 0.5f; // 高速域: 重くなる
	return 0.5f;                                                // 超高速
}


// ===========================================================
// 合力の計算（全面改修版）
// ===========================================================
MyMath::Vector3 FlightModel::CalculateTotalForce(float deltaTime)
{
	(void)deltaTime;

	MyMath::Vector3 totalForce = { 0.0f, 0.0f, 0.0f };
	float speed = MyMath::Length(velocity_);
	float altitude = position_.y;

	// 高度依存の空気密度 (ISA簡易モデル)
	float airDensity = CalculateAirDensity(altitude);

	// 動圧 q = 0.5 * ρ * v²
	float dynamicPressure = 0.5f * airDensity * speed * speed;
	float wingArea = airframe_.GetWingArea();


	// --- 1. 推力（高度 + エンジンダメージ補正） ---
	float engineDamage = airframe_.GetDamageState(DamageZone::Engine);
	float thrustMag = engine_.GetThrustAtAltitude(altitude, engineDamage);
	MyMath::Vector3 forward = GetForwardDirection();
	MyMath::Vector3 thrust = MyMath::Multiply(thrustMag, forward);
	totalForce = MyMath::Add(totalForce, thrust);


	// --- 2. 重力（常に下方向に作用） ---
	float totalMass = GetTotalMass();
	MyMath::Vector3 gravity = { 0.0f, -kGravity * totalMass, 0.0f };
	totalForce = MyMath::Add(totalForce, gravity);


	// --- 3. 揚力（AoA依存 + 翼ダメージ + フラップ補正） ---
	if (speed > 1.0f) {
		float cl = CalculateLiftCoefficient(currentAoA_);

		// フラップによるCL増加
		cl += airframe_.GetFlapPosition() * airframe_.GetFlapLiftBonus();

		// 翼ダメージ補正
		float wingDmgL = airframe_.GetDamageState(DamageZone::LeftWing);
		float wingDmgR = airframe_.GetDamageState(DamageZone::RightWing);
		float wingDamageFactor = 1.0f - (wingDmgL + wingDmgR) * 0.5f * 0.8f;
		cl *= wingDamageFactor;

		float liftMag = dynamicPressure * cl * wingArea;
		MyMath::Vector3 up = GetUpDirection();
		MyMath::Vector3 lift = MyMath::Multiply(liftMag, up);
		totalForce = MyMath::Add(totalForce, lift);


		// --- 4. 誘導抵抗（揚力の副産物） ---
		float ar = airframe_.GetAspectRatio();
		float e = airframe_.GetOswaldEfficiency();
		if (ar > 0.0f && e > 0.0f) {
			float inducedCd = (cl * cl) / (kPi * ar * e);
			float inducedDragMag = dynamicPressure * inducedCd * wingArea;
			MyMath::Vector3 velDir = MyMath::Normalize(velocity_);
			MyMath::Vector3 inducedDrag = MyMath::Multiply(-inducedDragMag, velDir);
			totalForce = MyMath::Add(totalForce, inducedDrag);
		}
	}


	// --- 5. 寄生抵抗（基本 + フラップ + エアブレーキ + ペイロード + ダメージ） ---
	if (speed > 0.001f) {
		float totalCd = airframe_.GetEffectiveBaseDrag();

		// フラップの追加抵抗
		totalCd += airframe_.GetFlapPosition() * airframe_.GetFlapDragBonus();

		// エアブレーキの追加抵抗
		totalCd += airframe_.GetAirBrakePosition() * airframe_.GetAirBrakeDragBonus();

		// ペイロードの追加抵抗
		for (const auto& payload : payloads_) {
			totalCd += payload->GetDragCoeff();
		}

		float parasiteDragMag = dynamicPressure * totalCd * wingArea;
		MyMath::Vector3 velDir = MyMath::Normalize(velocity_);
		MyMath::Vector3 parasiteDrag = MyMath::Multiply(-parasiteDragMag, velDir);
		totalForce = MyMath::Add(totalForce, parasiteDrag);
	}

	return totalForce;
}


// ===========================================================
// 姿勢（クォータニオン）の更新（速度依存 + ダメージ連動版）
// ===========================================================
void FlightModel::UpdateOrientation(float deltaTime)
{
	float speed = MyMath::Length(velocity_);

	// 速度依存の操舵効率
	float controlEff = CalculateControlEfficiency(speed);

	// ダメージによる操舵低下
	controlEff *= airframe_.GetControlDamageFactor();

	// ブラックアウト/レッドアウト中の操舵低下
	float gPenalty = 1.0f - (std::max)(blackoutFactor_, redoutFactor_) * 0.8f;
	controlEff *= gPenalty;

	// 各軸の回転角度（入力 × 角速度 × 操舵効率 × dt）
	float pitchAngle = inputPitch_ * kPitchRate * controlEff * deltaTime;
	float rollAngle = inputRoll_ * kRollRate * controlEff * deltaTime;
	float yawAngle = inputYaw_ * kYawRate * controlEff * deltaTime;

	// ローカル軸の取得
	MyMath::Vector3 right = GetRightDirection();
	MyMath::Vector3 up = GetUpDirection();
	MyMath::Vector3 forward = GetForwardDirection();

	// 各軸回りの微小回転クォータニオンを生成して合成
	// q = cos(θ/2) + sin(θ/2) * axis
	auto makeRotation = [](const MyMath::Vector3& axis, float angle) -> MyMath::Quaternion {
		float halfAngle = angle * 0.5f;
		float s = std::sin(halfAngle);
		float c = std::cos(halfAngle);
		return { axis.x * s, axis.y * s, axis.z * s, c };
	};

	// クォータニオンの乗算
	auto qMul = [](const MyMath::Quaternion& a, const MyMath::Quaternion& b) -> MyMath::Quaternion {
		return {
			a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
			a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
			a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
			a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
		};
	};

	// ピッチ → ヨー → ロール の順に回転を適用
	MyMath::Quaternion qPitch = makeRotation(right, pitchAngle);
	MyMath::Quaternion qYaw = makeRotation(up, yawAngle);
	MyMath::Quaternion qRoll = makeRotation(forward, rollAngle);

	// 合成: orientation = qRoll * qYaw * qPitch * orientation
	MyMath::Quaternion combined = qMul(qRoll, qMul(qYaw, qPitch));
	orientation_ = qMul(combined, orientation_);

	// クォータニオンの正規化（数値誤差の蓄積を防止）
	float len = std::sqrt(
		orientation_.x * orientation_.x +
		orientation_.y * orientation_.y +
		orientation_.z * orientation_.z +
		orientation_.w * orientation_.w
	);
	if (len > 0.0001f) {
		float invLen = 1.0f / len;
		orientation_.x *= invLen;
		orientation_.y *= invLen;
		orientation_.z *= invLen;
		orientation_.w *= invLen;
	}
}


// ===========================================================
// G効果の更新（ブラックアウト・レッドアウト + 構造ダメージ）
// ===========================================================
void FlightModel::UpdateGEffects(float deltaTime)
{
	// --- ブラックアウト（高正G） ---
	if (currentG_ > kBlackoutOnsetG) {
		float excess = currentG_ - kBlackoutOnsetG;
		blackoutFactor_ += excess * kGEffectBuildRate * deltaTime;
	} else {
		blackoutFactor_ -= kGEffectRecoveryRate * deltaTime;
	}
	blackoutFactor_ = std::clamp(blackoutFactor_, 0.0f, 1.0f);

	// --- レッドアウト（負G） ---
	if (currentG_ < kRedoutOnsetG) {
		float excess = -(currentG_ - kRedoutOnsetG); // 正の値にする
		redoutFactor_ += excess * (kGEffectBuildRate * 1.5f) * deltaTime;
	} else {
		redoutFactor_ -= kGEffectRecoveryRate * deltaTime;
	}
	redoutFactor_ = std::clamp(redoutFactor_, 0.0f, 1.0f);

	// --- 構造G超過ダメージ（翼に蓄積） ---
	float posLimit = airframe_.GetPositiveGLimit();
	float negLimit = airframe_.GetNegativeGLimit();

	if (currentG_ > posLimit) {
		float overG = currentG_ - posLimit;
		airframe_.ApplyZoneDamage(DamageZone::LeftWing, overG * kGDamageRate * deltaTime);
		airframe_.ApplyZoneDamage(DamageZone::RightWing, overG * kGDamageRate * deltaTime);
	}
	if (currentG_ < negLimit) {
		float overG = negLimit - currentG_;
		airframe_.ApplyZoneDamage(DamageZone::LeftWing, overG * kGDamageRate * deltaTime);
		airframe_.ApplyZoneDamage(DamageZone::RightWing, overG * kGDamageRate * deltaTime);
	}
}


// ===========================================================
// 方向ベクトルの取得（クォータニオンから計算）
// ===========================================================
MyMath::Vector3 FlightModel::GetForwardDirection() const
{
	// クォータニオンで (0,0,1) を回転させた結果
	float x = orientation_.x;
	float y = orientation_.y;
	float z = orientation_.z;
	float w = orientation_.w;

	return {
		2.0f * (x * z + w * y),
		2.0f * (y * z - w * x),
		1.0f - 2.0f * (x * x + y * y)
	};
}

MyMath::Vector3 FlightModel::GetUpDirection() const
{
	// クォータニオンで (0,1,0) を回転させた結果
	float x = orientation_.x;
	float y = orientation_.y;
	float z = orientation_.z;
	float w = orientation_.w;

	return {
		2.0f * (x * y - w * z),
		1.0f - 2.0f * (x * x + z * z),
		2.0f * (y * z + w * x)
	};
}

MyMath::Vector3 FlightModel::GetRightDirection() const
{
	// クォータニオンで (1,0,0) を回転させた結果
	float x = orientation_.x;
	float y = orientation_.y;
	float z = orientation_.z;
	float w = orientation_.w;

	return {
		1.0f - 2.0f * (y * y + z * z),
		2.0f * (x * y + w * z),
		2.0f * (x * z - w * y)
	};
}