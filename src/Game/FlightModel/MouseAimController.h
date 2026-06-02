#pragma once
#include "math/MyMath.h"

class FlightModel;

/// 汎用PIDコントローラー（1軸分）
struct PIDController {
	float kP = 0.0f;    // 比例ゲイン
	float kI = 0.0f;    // 積分ゲイン
	float kD = 0.0f;    // 微分ゲイン

	float integral = 0.0f;      // 積分値
	float prevError = 0.0f;     // 前フレームの誤差
	float maxIntegral = 1.0f;   // 積分アンチワインドアップ上限

	/// PIDのパラメータを設定
	void SetGains(float p, float i, float d, float maxI = 1.0f) {
		kP = p; kI = i; kD = d; maxIntegral = maxI;
	}

	/// 誤差と経過時間からPID出力を計算
	float Update(float error, float dt) {
		// 比例項
		float pTerm = kP * error;

		// 積分項（台形法 + アンチワインドアップ）
		integral += error * dt;
		if (integral > maxIntegral) integral = maxIntegral;
		if (integral < -maxIntegral) integral = -maxIntegral;
		float iTerm = kI * integral;

		// 微分項（誤差の変化率）
		float derivative = 0.0f;
		if (dt > 0.0001f) {
			derivative = (error - prevError) / dt;
		}
		float dTerm = kD * derivative;

		prevError = error;

		return pTerm + iTerm + dTerm;
	}

	/// 状態をリセット
	void Reset() {
		integral = 0.0f;
		prevError = 0.0f;
	}
};


/// War Thunder風 マウスエイム・コントローラー
/// マウスカーソル位置 → 目標方向 → PID制御で操舵入力を生成。
class MouseAimController {
public:
	MouseAimController() = default;
	~MouseAimController() = default;

	/// 初期化（スクリーンサイズを設定）
	void Initialize(float screenWidth, float screenHeight);

	/// マウスの相対移動量を受け取り、仮想カーソル位置を更新
	void UpdateCursorPosition(long deltaX, long deltaY);

	/// 仮想カーソル位置をリセット（画面中央に戻す）
	void ResetCursor();

	/// 仮想カーソル位置とカメラ情報から目標方向を計算し、
	/// PID制御で操舵入力を生成する。
	/// @param orientation 機体の現在姿勢（クォータニオン）
	/// @param cameraYaw カメラの現在のヨー角（ラジアン）
	/// @param cameraPitch カメラの現在のピッチ角（ラジアン）
	/// @param deltaTime フレーム間の経過時間（秒）
	/// @param outPitch 生成されたピッチ入力 (-1 ～ 1)
	/// @param outRoll  生成されたロール入力 (-1 ～ 1)
	/// @param outYaw   生成されたヨー入力 (-1 ～ 1)
	void CalculateSteeringInput(
		const MyMath::Quaternion& orientation,
		float cameraYaw, float cameraPitch,
		float deltaTime,
		float& outPitch, float& outRoll, float& outYaw
	);

	/// 有効/無効の切り替え
	void SetEnabled(bool enabled) { enabled_ = enabled; }
	bool IsEnabled() const { return enabled_; }
	void ToggleEnabled() { enabled_ = !enabled_; }

	/// マウス感度の設定
	void SetSensitivity(float sensitivity) { sensitivity_ = sensitivity; }
	float GetSensitivity() const { return sensitivity_; }

	/// 仮想カーソル位置の取得（スクリーン座標）
	float GetCursorX() const { return cursorX_; }
	float GetCursorY() const { return cursorY_; }

	/// 目標方向ベクトルの取得（デバッグ用）
	MyMath::Vector3 GetTargetDirection() const { return targetDirection_; }

	/// PIDコントローラーへのアクセス（ImGuiチューニング用）
	PIDController& GetPitchPID() { return pitchPID_; }
	PIDController& GetRollPID() { return rollPID_; }
	PIDController& GetYawPID() { return yawPID_; }
	PIDController& GetBankPID() { return bankPID_; }

private:
	bool enabled_ = true;

	// スクリーンサイズ
	float screenWidth_ = 1280.0f;
	float screenHeight_ = 720.0f;

	// 仮想カーソル位置（スクリーン座標）
	float cursorX_ = 640.0f;   // 画面中央
	float cursorY_ = 360.0f;

	// マウス感度（ピクセル → 角度の変換係数）
	float sensitivity_ = 0.15f;

	// 最大カーソル移動範囲（画面端に対する割合）
	float maxCursorRange_ = 0.85f;

	// 計算済みの目標方向（キャッシュ）
	MyMath::Vector3 targetDirection_ = { 0.0f, 0.0f, 1.0f };

	// コーディネートターンの閾値（ラジアン）
	float coordinatedTurnThreshold_ = 0.26f;  // ≈ 15度

	// === PIDコントローラー（各軸） ===
	PIDController pitchPID_;   // ピッチ誤差 → ピッチ入力
	PIDController rollPID_;    // バンク誤差 → ロール入力（コーディネートターン時）
	PIDController yawPID_;     // ヨー誤差 → ヨー入力（小角度時）
	PIDController bankPID_;    // 水平復帰用ロール

	// ヘルパー関数
	MyMath::Vector3 GetForwardFromOrientation(const MyMath::Quaternion& q) const;
	MyMath::Vector3 GetUpFromOrientation(const MyMath::Quaternion& q) const;
	MyMath::Vector3 GetRightFromOrientation(const MyMath::Quaternion& q) const;
};
