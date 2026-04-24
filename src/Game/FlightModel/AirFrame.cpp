#include "AirFrame.h"
#include <algorithm>
void Airframe::Initialize(const AirframeData& data) {
	emptyFrameMass_ = data.emptyFrameMass;
	maxInternalFuel_ = data.maxInternalFuel;
	baseDrag_ = data.baseDrag;
	liftCoefficient_ = data.liftCoefficient;
	currentInternalFuel_ = maxInternalFuel_; // 初期状態では満タン
}

void Airframe::ConsumeInternalFuel(float amount) {
	currentInternalFuel_ -= amount;
	currentInternalFuel_ = (std::max)(currentInternalFuel_, 0.0f);
}

float Airframe::GetTotalMass() const {
	return emptyFrameMass_ + currentInternalFuel_;
}