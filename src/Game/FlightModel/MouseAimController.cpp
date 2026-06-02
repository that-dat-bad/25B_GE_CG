#include "MouseAimController.h"
#include <cmath>
#include <algorithm>

namespace {
	constexpr float kPi = 3.14159265358979f;

	// カーソル位置を角度に変換するときの最大角度（ラジアン）
	constexpr float kMaxYawOffset = 1.2f;    // ≈ 70度
	constexpr float kMaxPitchOffset = 0.8f;  // ≈ 45度

	// デッドゾーン（この角度以下の差分は無視）
	constexpr float kDeadZone = 0.005f;      // ≈ 0.3度

	// バンク角の最大値（コーディネートターン時）
	constexpr float kMaxBankAngle = 1.2f;    // ≈ 70度
}


// ===========================================================
// 初期化
// ===========================================================
void MouseAimController::Initialize(float screenWidth, float screenHeight)
{
	screenWidth_ = screenWidth;
	screenHeight_ = screenHeight;
	cursorX_ = screenWidth * 0.5f;
	cursorY_ = screenHeight * 0.5f;
	targetDirection_ = { 0.0f, 0.0f, 1.0f };

	// --- PIDゲインの初期設定 ---
	// ピッチPID: やや強めのP、振動抑制のD、定常偏差除去の弱いI
	pitchPID_.SetGains(2.5f, 0.3f, 0.8f, 0.5f);

	// ロールPID（コーディネートターン時のバンク制御）: 素早い応答のP、安定化のD
	rollPID_.SetGains(2.0f, 0.1f, 0.6f, 0.3f);

	// ヨーPID（小角度修正用）: 直接的なヨー補正
	yawPID_.SetGains(2.0f, 0.2f, 0.5f, 0.3f);

	// バンク水平復帰PID: 穏やかに水平へ戻す
	bankPID_.SetGains(1.0f, 0.1f, 0.4f, 0.3f);
}


// ===========================================================
// マウス相対移動量でカーソル位置を更新
// ===========================================================
void MouseAimController::UpdateCursorPosition(long deltaX, long deltaY)
{
	if (!enabled_) return;

	cursorX_ += static_cast<float>(deltaX) * sensitivity_;
	cursorY_ += static_cast<float>(deltaY) * sensitivity_;

	// 画面内にクランプ
	float margin = screenWidth_ * (1.0f - maxCursorRange_) * 0.5f;
	cursorX_ = std::clamp(cursorX_, margin, screenWidth_ - margin);
	cursorY_ = std::clamp(cursorY_, margin, screenHeight_ - margin);
}


// ===========================================================
// カーソル位置を画面中央にリセット
// ===========================================================
void MouseAimController::ResetCursor()
{
	cursorX_ = screenWidth_ * 0.5f;
	cursorY_ = screenHeight_ * 0.5f;

	// PIDの内部状態もリセット
	pitchPID_.Reset();
	rollPID_.Reset();
	yawPID_.Reset();
	bankPID_.Reset();
}


// ===========================================================
// PID制御による操舵入力の計算
// ===========================================================
void MouseAimController::CalculateSteeringInput(
	const MyMath::Quaternion& orientation,
	float cameraYaw, float cameraPitch,
	float deltaTime,
	float& outPitch, float& outRoll, float& outYaw
)
{
	if (!enabled_ || deltaTime < 0.0001f) {
		outPitch = 0.0f;
		outRoll = 0.0f;
		outYaw = 0.0f;
		return;
	}

	// =====================================================
	// 1. 仮想カーソル位置 → 目標方向ベクトル
	// =====================================================
	float normalizedX = (cursorX_ - screenWidth_ * 0.5f) / (screenWidth_ * 0.5f);
	float normalizedY = (cursorY_ - screenHeight_ * 0.5f) / (screenHeight_ * 0.5f);

	// LookAtRotation の規約: pitch = -asin(dir.y)
	float yawOffset = normalizedX * kMaxYawOffset;
	float pitchOffset = normalizedY * kMaxPitchOffset;

	float targetYaw = cameraYaw + yawOffset;
	float targetPitch = cameraPitch + pitchOffset;

	// LookAtRotation: pitch = -asin(dir.y) → dir.y = -sin(pitch)
	targetDirection_ = {
		std::cos(targetPitch) * std::sin(targetYaw),
		-std::sin(targetPitch),
		std::cos(targetPitch) * std::cos(targetYaw)
	};

	// =====================================================
	// 2. 機体前方方向と目標方向の差分を計算
	// =====================================================
	MyMath::Vector3 forward = GetForwardFromOrientation(orientation);
	MyMath::Vector3 up = GetUpFromOrientation(orientation);
	MyMath::Vector3 right = GetRightFromOrientation(orientation);

	// 目標方向を機体のローカル座標系に射影
	float localPitchError = MyMath::Dot(targetDirection_, up);
	float localYawError = MyMath::Dot(targetDirection_, right);
	float forwardDot = MyMath::Dot(targetDirection_, forward);

	// 角度差分の推定
	float pitchAngleError = std::asin(std::clamp(localPitchError, -1.0f, 1.0f));
	float yawAngleError = std::asin(std::clamp(localYawError, -1.0f, 1.0f));

	// 目標が後方にある場合の補正
	if (forwardDot < 0.0f) {
		if (yawAngleError > 0.0f) {
			yawAngleError = kPi - yawAngleError;
		} else {
			yawAngleError = -kPi - yawAngleError;
		}
	}

	// =====================================================
	// 3. PID制御による操舵入力生成
	// =====================================================
	float absYawError = std::fabs(yawAngleError);

	if (absYawError < kDeadZone && std::fabs(pitchAngleError) < kDeadZone) {
		outPitch = 0.0f;
		outRoll = 0.0f;
		outYaw = 0.0f;
		// 積分値を徐々に減衰（デッドゾーン内で急にリセットしない）
		pitchPID_.integral *= 0.95f;
		yawPID_.integral *= 0.95f;
		rollPID_.integral *= 0.95f;
		return;
	}

	// 現在のバンク角の推定
	// bankIndicator: 正 = 左バンク, 負 = 右バンク（ロール入力と同じ符号規約）
	MyMath::Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
	float currentBankIndicator = MyMath::Dot(right, worldUp);

	if (absYawError > coordinatedTurnThreshold_) {
		// =============================================
		// コーディネートターン: バンク＆プル
		// =============================================

		// 目標バンク角: 目標が右(yawError>0) → 右バンク(負)
		float targetBank = std::clamp(-yawAngleError * 1.5f, -kMaxBankAngle, kMaxBankAngle);

		// バンク誤差をPIDで制御 → ロール入力
		float bankError = targetBank - currentBankIndicator;
		float rollOutput = rollPID_.Update(bankError, deltaTime);
		outRoll = std::clamp(rollOutput, -1.0f, 1.0f);

		// バンク度合いに応じた引き起こし
		float bankRatio = std::fabs(currentBankIndicator) / kMaxBankAngle;
		bankRatio = std::clamp(bankRatio, 0.0f, 1.0f);

		// ピッチ: PIDで制御 + バンク時の引き起こし
		float pitchOutput = pitchPID_.Update(pitchAngleError, deltaTime);
		float pullUp = bankRatio * 0.8f;
		outPitch = std::clamp(-(pullUp + pitchOutput * (1.0f - bankRatio * 0.5f)), -1.0f, 1.0f);

		// ヨー: 補助ラダー（PIDの比例項のみ使用、弱め）
		outYaw = std::clamp(yawAngleError * yawPID_.kP * 0.3f, -1.0f, 1.0f);

	} else {
		// =============================================
		// 小角度修正: 各軸PIDで直接制御
		// =============================================

		// ピッチ: PID制御
		float pitchOutput = pitchPID_.Update(pitchAngleError, deltaTime);
		outPitch = std::clamp(-pitchOutput, -1.0f, 1.0f);

		// ヨー: PID制御
		float yawOutput = yawPID_.Update(yawAngleError, deltaTime);
		outYaw = std::clamp(yawOutput, -1.0f, 1.0f);

		// ロール: バンク角を0に戻すPID制御
		// 誤差 = 0(水平) - 現在のバンク = -currentBankIndicator
		float levelError = -currentBankIndicator;
		float bankOutput = bankPID_.Update(levelError, deltaTime);
		outRoll = std::clamp(bankOutput, -1.0f, 1.0f);
	}
}


// ===========================================================
// ヘルパー: クォータニオンから方向ベクトルを取得
// ===========================================================
MyMath::Vector3 MouseAimController::GetForwardFromOrientation(const MyMath::Quaternion& q) const
{
	float x = q.x, y = q.y, z = q.z, w = q.w;
	return {
		2.0f * (x * z + w * y),
		2.0f * (y * z - w * x),
		1.0f - 2.0f * (x * x + y * y)
	};
}

MyMath::Vector3 MouseAimController::GetUpFromOrientation(const MyMath::Quaternion& q) const
{
	float x = q.x, y = q.y, z = q.z, w = q.w;
	return {
		2.0f * (x * y - w * z),
		1.0f - 2.0f * (x * x + z * z),
		2.0f * (y * z + w * x)
	};
}

MyMath::Vector3 MouseAimController::GetRightFromOrientation(const MyMath::Quaternion& q) const
{
	float x = q.x, y = q.y, z = q.z, w = q.w;
	return {
		1.0f - 2.0f * (y * y + z * z),
		2.0f * (x * y + w * z),
		2.0f * (x * z - w * y)
	};
}
