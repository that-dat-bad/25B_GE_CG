#pragma once

// --- 外部ファイルから読み込むためのデータ構造体（DTO） ---
struct AirframeData {
	float emptyFrameMass;   // 骨組みのみの空虚重量 (kg)
	float maxInternalFuel;  // 内蔵燃料の最大容量 (kg)
	float baseDrag;         // 基本の空気抵抗係数
	float liftCoefficient;  // 揚力係数（翼の性能）
};

// --- 機体フレームクラス本体 ---
class Airframe {
public:
	Airframe() : currentInternalFuel_(0.0f) {}
	~Airframe() = default;

	// 初期化（外部データを受け取ってセットアップ）
	void Initialize(const AirframeData& data);

	// 内蔵燃料を消費する処理
	void ConsumeInternalFuel(float amount);

	// --- アクセッサ（外部から情報を取得） ---
	float GetTotalMass() const;

	float GetCurrentInternalFuel() const { return currentInternalFuel_; }
	float GetMaxInternalFuel() const { return maxInternalFuel_; }
	float GetBaseDrag() const { return baseDrag_; }
	float GetLiftCoefficient() const { return liftCoefficient_; }

private:
	// --- 物理スペック（不変データ） ---
	float emptyFrameMass_;      // 骨組みのみの空虚重量 (kg)
	float maxInternalFuel_;     // 内蔵燃料の最大容量 (kg)
	float baseDrag_;            // 基本空気抵抗係数
	float liftCoefficient_;     // 揚力係数

	// --- 現在の状態（変動データ） ---
	float currentInternalFuel_; // 現在の内蔵燃料 (kg)
};