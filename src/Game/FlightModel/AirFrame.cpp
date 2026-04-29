#include "AirFrame.h"
#include <algorithm>

void Airframe::Initialize(const AirframeData& data) {
	emptyFrameMass_ = data.emptyFrameMass;
	maxInternalFuel_ = data.maxInternalFuel;
	baseDrag_ = data.baseDrag;
	liftCoefficient_ = data.liftCoefficient;
	wingArea_ = data.wingArea;
	maxHealth_ = data.maxHealth;

	currentInternalFuel_ = maxInternalFuel_; // 初期状態では満タン
	currentHealth_ = maxHealth_;             // 初期状態ではフル耐久
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