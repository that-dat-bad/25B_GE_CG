#pragma once

#include <vector>
#include <memory>
#include "math/MyMath.h"

#include "AirFrame.h"
#include "Engine.h"
#include "Payload/IPayload.h"


class FlightModel {
public:
	FlightModel();
	~FlightModel() = default;

	// 初期化
	void Initialize(const AirframeData& airframeData, const EngineData& engineData);

	// 毎フレームの更新処理
	void Update(float deltaTime);


	// -- ペイロード（外部武装・タンク等）の管理 --
	void AddPayload(std::unique_ptr<IPayload> payload);
	void FireWeapons(); // トリガーを引いた時用


	// ---setter --
	void SetThrottleInput(float throttle) { targetThrottle_ = throttle; }                      // 0.0f(停止) ~ 1.0f(全開)
	void SetControlInput(float pitch, float roll, float yaw) { inputPitch_ = pitch; inputRoll_ = roll; inputYaw_ = yaw; }   // 各軸 -1.0f ~ 1.0f


	// -- getter --
	MyMath::Vector3 GetPosition() const { return position_; }
	MyMath::Vector3 GetVelocity() const { return velocity_; }
	MyMath::Quaternion GetOrientation() const { return orientation_; }
	float GetSpeed() const { return MyMath::Length(velocity_); }
	float GetTotalMass() const;
	float GetCurrentThrottle() const { return engine_.GetCurrentThrottle(); }
	float GetCurrentFuel() const { return airframe_.GetCurrentInternalFuel(); }

	// 外部から位置・姿勢をセットする（スポーン時など）
	void SetPosition(const MyMath::Vector3& pos) { position_ = pos; }
	void SetOrientation(const MyMath::Quaternion& ori) { orientation_ = ori; }

	// 方向ベクトルの取得（クォータニオンから計算）
	MyMath::Vector3 GetForwardDirection() const;
	MyMath::Vector3 GetUpDirection() const;
	MyMath::Vector3 GetRightDirection() const;


private:
	// コンポーネント
	Airframe airframe_;
	Engine   engine_;

	// 動的に着脱されるペイロード群（ミサイル、ガンポッドなど）
	std::vector<std::unique_ptr<IPayload>> payloads_;


	// 物理ステータス
	MyMath::Vector3 position_;      // 座標
	MyMath::Vector3 velocity_;      // 速度
	MyMath::Vector3 acceleration_;  // 加速度
	MyMath::Quaternion orientation_; // 姿勢（回転）


	// プレイヤーからの入力状態
	float targetThrottle_;  // 目標スロットル
	float inputPitch_;      // ピッチ入力
	float inputRoll_;       // ロール入力
	float inputYaw_;        // ヨー入力


	// ヘルパー関数
	MyMath::Vector3 CalculateTotalForce(float deltaTime);
	void UpdateOrientation(float deltaTime);

};