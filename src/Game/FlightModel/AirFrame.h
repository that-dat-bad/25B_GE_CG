#pragma once
#include <algorithm>

// --- 外部ファイルから読み込むためのデータ構造体（DTO） ---
struct AirframeData {
	float emptyFrameMass;   // 骨組みのみの空虚重量 (kg)
	float maxInternalFuel;  // 内蔵燃料の最大容量 (kg)
	float baseDrag;         // 基本の空気抵抗係数
	float liftCoefficient;  // 揚力係数（翼の性能）
	float wingArea;         // 翼面積 (m^2)
	float maxHealth;        // 最大耐久値

	// --- 揚力・失速 ---
	float criticalAoA;          // 臨界迎え角（rad）
	float maxLiftCoefficient;   // 臨界AoA時の最大CL
	float stallLiftCoefficient; // 失速後の残留CL

	// --- 誘導抵抗 ---
	float aspectRatio;          // 翼アスペクト比
	float oswaldEfficiency;     // オズワルド効率

	// --- G制限 ---
	float positiveGLimit;       // +Gリミット
	float negativeGLimit;       // -Gリミット

	// --- フラップ ---
	float flapLiftBonus;        // フラップ展開時のCL増加量
	float flapDragBonus;        // フラップ展開時のCd増加量
	float flapMaxSpeed;         // フラップが使用可能な最大速度 (m/s)
	float flapDeploySpeed;      // フラップ展開/収納速度 (0→1の速度/sec)

	// --- エアブレーキ ---
	float airBrakeDragBonus;    // エアブレーキ展開時のCd増加量
	float airBrakeDeploySpeed;  // エアブレーキ展開/収納速度
};

// --- ダメージ部位 ---
enum class DamageZone {
	Engine,
	LeftWing,
	RightWing,
	Tail,
	FuelTank,
	Count
};

// --- 機体フレームクラス本体 ---
class Airframe {
public:
	Airframe() : currentInternalFuel_(0.0f), currentHealth_(0.0f),
		flapPosition_(0.0f), flapDesired_(false),
		airBrakePosition_(0.0f), airBrakeDesired_(false) {
		for (int i = 0; i < static_cast<int>(DamageZone::Count); ++i) {
			damageState_[i] = 0.0f;
		}
	}
	~Airframe() = default;

	// 初期化（外部データを受け取ってセットアップ）
	void Initialize(const AirframeData& data);

	// 内蔵燃料を消費する処理
	void ConsumeInternalFuel(float amount);

	// ダメージを受ける処理
	void TakeDamage(float damage);

	// --- フラップ ---
	void SetFlapDesired(bool desired) { flapDesired_ = desired; }
	void UpdateFlap(float deltaTime, float speed);
	float GetFlapPosition() const { return flapPosition_; }
	bool  IsFlapOverspeed(float speed) const { return speed > flapMaxSpeed_; }
	float GetFlapLiftBonus() const { return flapLiftBonus_; }
	float GetFlapDragBonus() const { return flapDragBonus_; }

	// --- エアブレーキ ---
	void SetAirBrakeDesired(bool desired) { airBrakeDesired_ = desired; }
	void UpdateAirBrake(float deltaTime);
	float GetAirBrakePosition() const { return airBrakePosition_; }
	float GetAirBrakeDragBonus() const { return airBrakeDragBonus_; }

	// --- ダメージ連動 ---
	void ApplyZoneDamage(DamageZone zone, float amount);
	float GetDamageState(DamageZone zone) const { return damageState_[static_cast<int>(zone)]; }

	// ダメージ補正値
	float GetEffectiveLiftCoefficient() const;   // 翼ダメージで揚力低下
	float GetEffectiveBaseDrag() const;           // ダメージで抵抗増加
	float GetControlDamageFactor() const;         // 尾翼ダメージで操舵低下
	float GetFuelLeakRate() const;                // 燃料タンクダメージで燃料漏れ

	// --- アクセッサ（既存 + 新規） ---
	float GetTotalMass() const;

	float GetCurrentInternalFuel() const { return currentInternalFuel_; }
	float GetMaxInternalFuel() const { return maxInternalFuel_; }
	float GetBaseDrag() const { return baseDrag_; }
	float GetLiftCoefficient() const { return liftCoefficient_; }
	float GetWingArea() const { return wingArea_; }
	float GetCurrentHealth() const { return currentHealth_; }
	float GetMaxHealth() const { return maxHealth_; }
	bool  IsDestroyed() const { return currentHealth_ <= 0.0f; }

	// 揚力・失速
	float GetCriticalAoA() const { return criticalAoA_; }
	float GetMaxLiftCoefficient() const { return maxLiftCoefficient_; }
	float GetStallLiftCoefficient() const { return stallLiftCoefficient_; }

	// 誘導抵抗
	float GetAspectRatio() const { return aspectRatio_; }
	float GetOswaldEfficiency() const { return oswaldEfficiency_; }

	// G制限
	float GetPositiveGLimit() const { return positiveGLimit_; }
	float GetNegativeGLimit() const { return negativeGLimit_; }

private:
	// --- 物理スペック（不変データ） ---
	float emptyFrameMass_;      // 骨組みのみの空虚重量 (kg)
	float maxInternalFuel_;     // 内蔵燃料の最大容量 (kg)
	float baseDrag_;            // 基本空気抵抗係数
	float liftCoefficient_;     // 揚力係数
	float wingArea_;            // 翼面積 (m^2)
	float maxHealth_;           // 最大耐久値

	// 揚力・失速
	float criticalAoA_;
	float maxLiftCoefficient_;
	float stallLiftCoefficient_;

	// 誘導抵抗
	float aspectRatio_;
	float oswaldEfficiency_;

	// G制限
	float positiveGLimit_;
	float negativeGLimit_;

	// フラップ
	float flapLiftBonus_;
	float flapDragBonus_;
	float flapMaxSpeed_;
	float flapDeploySpeed_;

	// エアブレーキ
	float airBrakeDragBonus_;
	float airBrakeDeploySpeed_;

	// --- 現在の状態（変動データ） ---
	float currentInternalFuel_; // 現在の内蔵燃料 (kg)
	float currentHealth_;       // 現在の耐久値

	// フラップ状態
	float flapPosition_;        // 0.0 (収納) ～ 1.0 (全展開)
	bool  flapDesired_;

	// エアブレーキ状態
	float airBrakePosition_;    // 0.0 (収納) ～ 1.0 (全展開)
	bool  airBrakeDesired_;

	// ダメージ状態（各部位 0.0～1.0）
	float damageState_[static_cast<int>(DamageZone::Count)];
};