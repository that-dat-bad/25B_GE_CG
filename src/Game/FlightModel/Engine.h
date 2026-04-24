#pragma once

// --- 外部ファイルから読み込むためのデータ構造体（DTO） ---
struct EngineData {
	float mass;                 // エンジンの重量
	float baseThrust;           // 定格推力（スロットル1.0時の推力）
	float normalThrottleLimit;  // 通常のスロットル上限（1.0）
	float wepThrottleLimit;     // WEP時のスロットル上限k
	float physicalSpoolSpeed;   // エンジンの物理的な反応速度
	float baseFuelFlowRate;     // 定格推力時(1.0)の燃料消費量
};

// --- エンジンクラス本体 ---
class Engine {
public:
	Engine() : currentThrottle_(0.0f) {}
	~Engine() = default;

	// 初期化（外部データを受け取ってセットアップ）
	void Initialize(const EngineData& data);

	// 毎フレームの更新処理（目標スロットルに向かって物理的に出力を変化させる）
	void Update(float dt, float targetThrottle);

	// --- アクセッサ（外部から情報を取得） ---
	float GetMass() const { return mass_; }
	float GetCurrentThrust() const { return baseThrust_ * currentThrottle_; }
	float GetFuelConsumptionRate() const { return baseFuelFlowRate_ * currentThrottle_; }
	float GetCurrentThrottle() const { return currentThrottle_; }

private:
	// --- 物理スペック（不変データ） ---
	float mass_;                        // エンジン自体の重量
	float baseThrust_;                  // 定格推力（1.0時の推力）
	float normalThrottleLimit_;         // 通常スロットル上限（基本1.0）
	float wepThrottleLimit_;            // WEP時スロットル上限（例: 1.1）
	float enginePhysicalSpoolSpeed_;    // エンジンの物理的な反応速度
	float baseFuelFlowRate_;            // 定格推力時(1.0)の燃料消費率

	// --- 現在の状態（変動データ） ---
	float currentThrottle_;             // 現在の実際のエンジン出力（0.0 ～ WEP上限）
};