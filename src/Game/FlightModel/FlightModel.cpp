#include "FlightModel.h"
#include <cmath>

// --- 定数 ---
namespace {
	constexpr float kGravity = 9.81f;            // 重力加速度 (m/s^2)
	constexpr float kAirDensity = 1.225f;        // 海面気圧での空気密度 (kg/m^3)
	constexpr float kPitchRate = 1.5f;           // ピッチ角速度 (rad/s)
	constexpr float kRollRate = 2.5f;            // ロール角速度 (rad/s)
	constexpr float kYawRate = 0.8f;             // ヨー角速度 (rad/s)
}


// ===========================================================
// コンストラクタ
// ===========================================================
FlightModel::FlightModel()
	: position_{ 0.0f, 0.0f, 0.0f }
	, velocity_{ 0.0f, 0.0f, 0.0f }
	, acceleration_{ 0.0f, 0.0f, 0.0f }
	, orientation_{ 0.0f, 0.0f, 0.0f, 1.0f } // 単位クォータニオン（無回転）
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

	// 2. 燃料消費
	float fuelConsumption = engine_.GetFuelConsumptionRate() * deltaTime;
	airframe_.ConsumeInternalFuel(fuelConsumption);

	// 3. ペイロードの更新
	for (auto& payload : payloads_) {
		payload->Update(deltaTime);
	}

	// 4. 姿勢の更新（プレイヤー入力に基づく回転）
	UpdateOrientation(deltaTime);

	// 5. 合力を計算
	MyMath::Vector3 totalForce = CalculateTotalForce(deltaTime);

	// 6. 加速度 = 力 / 質量 (F = ma → a = F/m)
	float totalMass = GetTotalMass();
	if (totalMass > 0.0f) {
		float invMass = 1.0f / totalMass;
		acceleration_ = MyMath::Multiply(invMass, totalForce);
	}

	// 7. 速度の更新 (v += a * dt)
	velocity_ = MyMath::Add(velocity_, MyMath::Multiply(deltaTime, acceleration_));

	// 8. 位置の更新 (p += v * dt)
	position_ = MyMath::Add(position_, MyMath::Multiply(deltaTime, velocity_));
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
// 合力の計算
// ===========================================================
MyMath::Vector3 FlightModel::CalculateTotalForce(float deltaTime)
{
	(void)deltaTime; // 現在は未使用だが将来の拡張用に残す

	MyMath::Vector3 totalForce = { 0.0f, 0.0f, 0.0f };

	// --- 1. 推力（エンジンが生み出す力、機体前方方向に作用） ---
	MyMath::Vector3 forward = GetForwardDirection();
	MyMath::Vector3 thrust = MyMath::Multiply(engine_.GetCurrentThrust(), forward);
	totalForce = MyMath::Add(totalForce, thrust);

	// --- 2. 重力（常に下方向に作用） ---
	float totalMass = GetTotalMass();
	MyMath::Vector3 gravity = { 0.0f, -kGravity * totalMass, 0.0f };
	totalForce = MyMath::Add(totalForce, gravity);

	// --- 3. 空気抵抗（速度の反対方向に作用、速度の2乗に比例） ---
	float speed = MyMath::Length(velocity_);
	if (speed > 0.001f) {
		// 全ペイロードの空気抵抗係数を合算
		float totalDragCoeff = airframe_.GetBaseDrag();
		for (const auto& payload : payloads_) {
			totalDragCoeff += payload->GetDragCoeff();
		}

		// 抗力 = 0.5 * ρ * v^2 * Cd * A
		float dragMagnitude = 0.5f * kAirDensity * speed * speed * totalDragCoeff * airframe_.GetWingArea();

		// 速度の反対方向に適用
		MyMath::Vector3 velocityDir = MyMath::Normalize(velocity_);
		MyMath::Vector3 drag = MyMath::Multiply(-dragMagnitude, velocityDir);
		totalForce = MyMath::Add(totalForce, drag);
	}

	// --- 4. 揚力（翼が生み出す上向きの力、速度の2乗に比例） ---
	if (speed > 0.001f) {
		// 揚力 = 0.5 * ρ * v^2 * CL * A（機体の上方向に作用）
		float liftMagnitude = 0.5f * kAirDensity * speed * speed
			* airframe_.GetLiftCoefficient() * airframe_.GetWingArea();

		MyMath::Vector3 up = GetUpDirection();
		MyMath::Vector3 lift = MyMath::Multiply(liftMagnitude, up);
		totalForce = MyMath::Add(totalForce, lift);
	}

	return totalForce;
}


// ===========================================================
// 姿勢（クォータニオン）の更新
// ===========================================================
void FlightModel::UpdateOrientation(float deltaTime)
{
	// 各軸の回転角度（入力 × 角速度 × dt）
	float pitchAngle = inputPitch_ * kPitchRate * deltaTime;
	float rollAngle = inputRoll_ * kRollRate * deltaTime;
	float yawAngle = inputYaw_ * kYawRate * deltaTime;

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