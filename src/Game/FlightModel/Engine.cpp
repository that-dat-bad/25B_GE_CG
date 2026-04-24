#include "Engine.h"

// --- 初期化処理（外部のDTOデータを受け取って自分の変数に入れる） ---
void Engine::Initialize(const EngineData& data) {
	mass_ = data.mass;
	baseThrust_ = data.baseThrust;
	normalThrottleLimit_ = data.normalThrottleLimit;
	wepThrottleLimit_ = data.wepThrottleLimit;
	enginePhysicalSpoolSpeed_ = data.physicalSpoolSpeed;
	baseFuelFlowRate_ = data.baseFuelFlowRate;

	currentThrottle_ = 0.0f; // エンジン停止状態でスタート
}

// --- 毎フレームの更新処理（スロットルの物理的な遅延） ---
void Engine::Update(float dt, float targetThrottle) {
	// 1. 目標値が限界（WEP上限）を超えないように安全対策（クランプ）
	if (targetThrottle > wepThrottleLimit_) {
		targetThrottle = wepThrottleLimit_;
	}

	// 2. 物理的な反応速度に従って、現在のスロットルを目標値に近づける
	if (currentThrottle_ < targetThrottle) {
		// スロットルを上げる時（スプールアップ）
		currentThrottle_ += enginePhysicalSpoolSpeed_ * dt;

		// 上がりすぎたら目標値で止める
		if (currentThrottle_ > targetThrottle) {
			currentThrottle_ = targetThrottle;
		}
	} else if (currentThrottle_ > targetThrottle) {
		// スロットルを下げる時（スプールダウン）
		currentThrottle_ -= enginePhysicalSpoolSpeed_ * dt;

		// 下がりすぎたら目標値でピタッと止める
		if (currentThrottle_ < targetThrottle) {
			currentThrottle_ = targetThrottle;
		}
	}
}