#include "AirFrame.h"
#include <algorithm>
#include <cmath>

void Airframe::Initialize(const AirframeData& data) {
	// --- 既存パラメータ ---
	emptyFrameMass_ = data.emptyFrameMass;
	maxInternalFuel_ = data.maxInternalFuel;
	baseDrag_ = data.baseDrag;
	liftCoefficient_ = data.liftCoefficient;
	wingArea_ = data.wingArea;
	maxHealth_ = data.maxHealth;
	centerOfGravityZ_ = data.centerOfGravityZ;

	// --- 揚力・失速 ---
	criticalAoA_ = data.criticalAoA;
	maxLiftCoefficient_ = data.maxLiftCoefficient;
	stallLiftCoefficient_ = data.stallLiftCoefficient;

	// --- 誘導抵抗 ---
	aspectRatio_ = data.aspectRatio;
	oswaldEfficiency_ = data.oswaldEfficiency;

	// --- G制限 ---
	positiveGLimit_ = data.positiveGLimit;
	negativeGLimit_ = data.negativeGLimit;

	// --- フラップ ---
	flapLiftBonus_ = data.flapLiftBonus;
	flapDragBonus_ = data.flapDragBonus;
	flapMaxSpeed_ = data.flapMaxSpeed;
	flapDeploySpeed_ = data.flapDeploySpeed;

	// --- エアブレーキ ---
	airBrakeDragBonus_ = data.airBrakeDragBonus;
	airBrakeDeploySpeed_ = data.airBrakeDeploySpeed;

	// --- 状態の初期化 ---
	currentInternalFuel_ = maxInternalFuel_; // 初期状態では満タン
	currentHealth_ = maxHealth_;             // 初期状態ではフル耐久

	flapPosition_ = 0.0f;
	flapDesired_ = false;
	airBrakePosition_ = 0.0f;
	airBrakeDesired_ = false;

	for (int i = 0; i < static_cast<int>(DamageZone::Count); ++i) {
		damageState_[i] = 0.0f;
	}
}

void Airframe::ConsumeInternalFuel(float amount) {
	currentInternalFuel_ -= amount;
	currentInternalFuel_ = (std::max)(currentInternalFuel_, 0.0f);
}

void Airframe::TakeDamage(float damage) {
	currentHealth_ -= damage;
	currentHealth_ = (std::max)(currentHealth_, 0.0f);
}

float Airframe::GetTotalMass() const {
	return emptyFrameMass_ + currentInternalFuel_;
}


// ===========================================================
// フラップの更新
// ===========================================================
void Airframe::UpdateFlap(float deltaTime, float speed) {
	// 速度超過時は強制収納（フラップ保護）
	bool canDeploy = flapDesired_ && !IsFlapOverspeed(speed);

	if (canDeploy) {
		// 展開
		flapPosition_ += flapDeploySpeed_ * deltaTime;
		if (flapPosition_ > 1.0f) flapPosition_ = 1.0f;
	} else {
		// 収納
		flapPosition_ -= flapDeploySpeed_ * deltaTime;
		if (flapPosition_ < 0.0f) flapPosition_ = 0.0f;
	}
}


// ===========================================================
// エアブレーキの更新
// ===========================================================
void Airframe::UpdateAirBrake(float deltaTime) {
	if (airBrakeDesired_) {
		airBrakePosition_ += airBrakeDeploySpeed_ * deltaTime;
		if (airBrakePosition_ > 1.0f) airBrakePosition_ = 1.0f;
	} else {
		airBrakePosition_ -= airBrakeDeploySpeed_ * deltaTime;
		if (airBrakePosition_ < 0.0f) airBrakePosition_ = 0.0f;
	}
}


// ===========================================================
// 部位ダメージの適用
// ===========================================================
void Airframe::ApplyZoneDamage(DamageZone zone, float amount) {
	int idx = static_cast<int>(zone);
	if (idx < 0 || idx >= static_cast<int>(DamageZone::Count)) return;

	damageState_[idx] += amount;
	if (damageState_[idx] > 1.0f) damageState_[idx] = 1.0f;
}


// ===========================================================
// ダメージ補正値
// ===========================================================
float Airframe::GetEffectiveLiftCoefficient() const {
	// 左右翼の平均ダメージで揚力低下（完全破壊で80%低下）
	float wingDamage = (damageState_[static_cast<int>(DamageZone::LeftWing)]
		+ damageState_[static_cast<int>(DamageZone::RightWing)]) * 0.5f;
	return liftCoefficient_ * (1.0f - wingDamage * 0.8f);
}

float Airframe::GetEffectiveBaseDrag() const {
	// ダメージで抵抗増加（破損箇所の乱流）
	float totalDamage = 0.0f;
	for (int i = 0; i < static_cast<int>(DamageZone::Count); ++i) {
		totalDamage += damageState_[i];
	}
	// ダメージ1部位あたり最大5%の抵抗増加
	return baseDrag_ * (1.0f + totalDamage * 0.05f);
}

float Airframe::GetControlDamageFactor() const {
	// 尾翼ダメージで操舵効率低下（完全破壊で70%低下）
	float tailDamage = damageState_[static_cast<int>(DamageZone::Tail)];
	return 1.0f - tailDamage * 0.7f;
}

float Airframe::GetFuelLeakRate() const {
	// 燃料タンクダメージで燃料漏れ（完全破壊で毎秒2kg漏出）
	float tankDamage = damageState_[static_cast<int>(DamageZone::FuelTank)];
	return tankDamage * 2.0f;
}