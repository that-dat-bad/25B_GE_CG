#pragma once
#include "IScene.h"
#include <memory>
#include "Object3d.h"
#include "Skybox.h"
#include "FlightModel/FlightModel.h"
#include "FlightModel/FlightInstructor.h"
#include "FlightModel/MouseAimController.h"
#include "FlightModel/Payload/Gunpod.h"
#include "Enemy/EnemyManager.h"
#include "Bullet/BulletManager.h"

/// @brief ゲーム本編のステージシーン
class StageScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	// フライトモデル（物理）
	FlightModel flightModel_;

	// フライトインストラクター（自動水平復帰）
	FlightInstructor flightInstructor_;

	// 描画用 3Dオブジェクト（機体の見た目）
	std::unique_ptr<Object3d> aircraftObject_ = nullptr;

	// 地面テクスチャ
	uint32_t groundTextureIndex_ = 0;

	// 地面グリッド描画
	void DrawGround();

	// Skybox
	std::unique_ptr<Skybox> skybox_ = nullptr;

	// ============================
	// War Thunder風 追従カメラ
	// ============================
	MyMath::Vector3 cameraCurrentPos_;   // カメラの現在位置（補間済み）
	MyMath::Vector3 cameraLookTarget_;   // カメラの注視点（補間済み）

	// カメラパラメータ
	float cameraDistance_  = 20.0f;   // 機体からの距離
	float cameraHeight_   = 5.0f;    // 機体の上方向オフセット
	float cameraPosLag_   = 5.0f;    // 位置の追従速度（大きいほど速い）
	float cameraLookLag_  = 10.0f;   // 注視点の追従速度

	// カメラ更新
	void UpdateChaseCamera(float dt);

	// ============================
	// 自由視点カメラ（Cキー長押し）
	// ============================
	bool  freeViewActive_ = false;       // Cキー押下中フラグ
	float freeViewYaw_    = 0.0f;        // 自由視点ヨー角
	float freeViewPitch_  = 0.0f;        // 自由視点ピッチ角
	float freeViewDistance_ = 25.0f;     // 機体からの距離

	// ヘルパー：クォータニオンからオイラー角(XYZ)への変換
	static MyMath::Vector3 QuaternionToEuler(const MyMath::Quaternion& q);

	// ヘルパー：注視点からカメラの回転(オイラー角)を計算
	MyMath::Vector3 LookAtRotation(const MyMath::Vector3& from, const MyMath::Vector3& to) const;

	// ============================
	// マウスエイム操縦
	// ============================
	MouseAimController mouseAimController_;
	bool mouseAimEnabled_ = true;    // マウスエイムモード ON/OFF

	// カメラの回転角キャッシュ（マウスエイムの目標方向計算に使用）
	float cachedCameraYaw_ = 0.0f;
	float cachedCameraPitch_ = 0.0f;

	// ============================
	// 戦闘システム
	// ============================
	GunPod gunpod_;
	EnemyManager enemyManager_;
	BulletManager bulletManager_;

	// ゲーム状態
	bool isMissionCleared_ = false;
	bool isMissionFailed_ = false;

	// スロットル入力状態（メンバー変数化して初期化時にリセットできるようにする）
	float throttle_ = 0.0f;
};