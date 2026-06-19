#include "StageScene.h"
#include "CameraManager.h"
#include "Object3dCommon.h"
#include "SkyboxCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PrimitiveModel.h"
#include "WinApp.h"
#include "../../engine/Graphics/PostProcess/PostEffect.h"
#include "../../engine/Graphics/EffectManager.h"
#include "../../engine/Graphics/ParticleManager.h"
#include "../../engine/Physics/CollisionManager.h"
#include <cmath>
#include <algorithm>

#ifdef USE_IMGUI
#include "../../../external/imgui/imgui.h"
#endif

using namespace MyMath;

// 固定フレームレートの deltaTime (60FPS想定)
static constexpr float kDeltaTime = 1.0f / 60.0f;


void StageScene::Initialize() {
	sceneID = SCENE::STAGE;

	// ============================
	// FlightModel の初期化
	// ============================
	AirframeData airframeData{};
	airframeData.emptyFrameMass = 3000.0f;   // 3トン
	airframeData.maxInternalFuel = 800.0f;    // 800kg
	airframeData.baseDrag = 0.02f;            // 基本空気抵抗
	airframeData.liftCoefficient = 0.2f;      // 揚力係数
	airframeData.wingArea = 40.0f;            // 翼面積 20m^2
	airframeData.maxHealth = 100.0f;          // 耐久値
	// 揚力・失速
	airframeData.criticalAoA = 0.5f;         // 臨界迎え角
	airframeData.maxLiftCoefficient = 1.5f;   // 最大CL
	airframeData.stallLiftCoefficient = 0.3f; // 失速後CL
	// 誘導抵抗
	airframeData.aspectRatio = 6.0f;
	airframeData.oswaldEfficiency = 0.8f;
	// G制限
	airframeData.positiveGLimit = 9.0f;
	airframeData.negativeGLimit = -3.0f;
	// フラップ
	airframeData.flapLiftBonus = 0.5f;
	airframeData.flapDragBonus = 0.08f;
	airframeData.flapMaxSpeed = 97.0f;        // ≈350 km/h
	airframeData.flapDeploySpeed = 2.0f;
	// エアブレーキ
	airframeData.airBrakeDragBonus = 0.15f;
	airframeData.airBrakeDeploySpeed = 3.0f;

	EngineData engineData{};
	engineData.mass = 500.0f;                 // エンジン 500kg
	engineData.baseThrust = 50000.0f;         // 推力 50kN
	engineData.normalThrottleLimit = 1.0f;
	engineData.wepThrottleLimit = 1.1f;       // WEPで110%
	engineData.physicalSpoolSpeed = 0.5f;     // スプール速度
	engineData.baseFuelFlowRate = 2.0f;       // 燃料消費率 2kg/s @ 100%
	engineData.altitudeThrottleFactor = 0.00004f; // 高度推力低下率

	flightModel_.Initialize(airframeData, engineData);

	// 初期位置: 上空100mから開始
	flightModel_.SetPosition({ 0.0f, 100.0f, 0.0f });

	// 開始時の速度を250km/hに設定
	float initialSpeed = 250.0f / 3.6f; // m/s
	flightModel_.SetVelocity(Multiply(initialSpeed, flightModel_.GetForwardDirection()));

	// スロットルを60%に初期化（速度維持のため）
	throttle_ = 0.6f;
	flightModel_.SetThrottle(throttle_);

	// ============================
	// 描画オブジェクトの初期化
	// ============================

	// テクスチャのプリロード（モデルのマテリアルが参照するテクスチャを先に読み込む）
	TextureManager::GetInstance()->LoadTexture("assets/textures/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("assets/textures/circle2.png");

	// 機体モデル
	ModelManager::GetInstance()->LoadModel("Resources/planeplane.obj");
	aircraftObject_ = std::make_unique<Object3d>();
	aircraftObject_->Initialize(Object3dCommon::GetInstance());
	aircraftObject_->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	aircraftObject_->SetModel("Resources/planeplane.obj");
	aircraftObject_->GetModel()->SetEnvironmentCoefficient(0.4f); // 機体表面にスカイボックスの映り込み

	// 地面テクスチャ
	TextureManager::GetInstance()->LoadTexture("assets/textures/white1x1.png");
	groundTextureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/white1x1.png");


	// Skybox
	TextureManager::GetInstance()->LoadTexture("assets/textures/cedar_bridge_sunset_1_2k.dds");
	skybox_ = std::make_unique<Skybox>();
	skybox_->Initialize(SkyboxCommon::GetInstance());
	skybox_->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	uint32_t skyboxTexIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/cedar_bridge_sunset_1_2k.dds");
	skybox_->SetTextureIndex(skyboxTexIndex);
	Object3dCommon::GetInstance()->SetDefaultEnvTextureIndex(skyboxTexIndex);

	// ============================
	// ライティング設定（太陽光）
	// ============================
	DirectionalLight* dirLight = Object3dCommon::GetInstance()->GetDirectionalLightData();
	if (dirLight) {
		dirLight->color = { 1.0f, 0.95f, 0.85f, 1.0f };       // やや暖色の太陽光
		dirLight->direction = { 0.5f, -0.8f, 0.3f };           // 斜め上から
		dirLight->intensity = 1.2f;
	}
	// ライティングモデル設定
	Object3dCommon::GetInstance()->SetLightType(1);             // Directional Light 有効
	Object3dCommon::GetInstance()->SetShadingModel(1);          // Half-Lambert
	Object3dCommon::GetInstance()->SetSpecularModel(2);         // Blinn-Phong

	// ============================
	// カメラ初期位置（機体の後方）
	// ============================
	Vector3 initPos = flightModel_.GetPosition();
	Vector3 initForward = flightModel_.GetForwardDirection();

	// 初期カメラ位置を即座にセット（補間の初期値）
	cameraCurrentPos_ = Add(initPos, Add(Multiply(-cameraDistance_, initForward), { 0.0f, cameraHeight_, 0.0f }));
	cameraLookTarget_ = initPos;

	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	if (camera) {
		camera->SetTranslate(cameraCurrentPos_);
		camera->SetRotate(LookAtRotation(cameraCurrentPos_, cameraLookTarget_));
	}
	CameraManager::GetInstance()->Update();

	// ============================
	// マウスエイムコントローラの初期化
	// ============================
	mouseAimController_.Initialize();
	mouseAimEnabled_ = true;

	// マウスエイム開始時はOSカーソルを非表示＆ロック
	Input::GetInstance()->LockCursor();

	// カメラの初期回転角をキャッシュ
	Vector3 camRot = LookAtRotation(cameraCurrentPos_, cameraLookTarget_);
	cachedCameraYaw_ = camRot.y;
	cachedCameraPitch_ = camRot.x;

	// ============================
	// 戦闘システムの初期化
	// ============================

	// Gunpod 初期化
	GunPodData gunpodData{};
	gunpodData.baseMass = 50.0f;
	gunpodData.drag = 0.005f;
	gunpodData.ammoWeight = 0.1f;
	gunpodData.maxAmmo = 200;
	gunpodData.fireRate = 10.0f;  // 秒間10発
	gunpod_.Initialize(gunpodData);

	// 敵の配置（ミッション01仕様）
	std::vector<EnemySpawnData> mission01Enemies = {
		{ {  100.0f, 0.0f,  200.0f }, "models/sphere.obj", 50.0f },
		{ {  -80.0f, 0.0f,  300.0f }, "models/sphere.obj", 50.0f },
		{ {  200.0f, 0.0f,  450.0f }, "models/sphere.obj", 50.0f },
		{ { -150.0f, 0.0f,  500.0f }, "models/sphere.obj", 50.0f },
		{ {   50.0f, 0.0f,  700.0f }, "models/sphere.obj", 50.0f },
	};
	enemyManager_.Initialize(mission01Enemies);

	// 弾丸マネージャー初期化
	bulletManager_.Initialize(512);

	// エフェクトマネージャー初期化
	EffectManager::GetInstance()->Initialize();
	ParticleManager::GetInstance()->CreateParticleGroup("HitSpark", "assets/textures/white1x1.png");

	// 爆発用パーティクルグループの登録
	TextureManager::GetInstance()->LoadTexture("assets/textures/circle.png");
	ParticleManager::GetInstance()->CreateParticleGroup("ExplosionSpark", "assets/textures/circle.png");
	ParticleManager::GetInstance()->SetBlendMode("ExplosionSpark", BlendMode::kAdd);

	ParticleManager::GetInstance()->CreateParticleGroup("ExplosionFire", "assets/textures/circle2.png");
	ParticleManager::GetInstance()->SetBlendMode("ExplosionFire", BlendMode::kAdd);

	ParticleManager::GetInstance()->CreateParticleGroup("ExplosionSmoke", "assets/textures/circle2.png");
	ParticleManager::GetInstance()->SetBlendMode("ExplosionSmoke", BlendMode::kNormal);

	// ゲーム状態リセット
	isMissionCleared_ = false;
	isMissionFailed_ = false;
}


void StageScene::Update() {
	// ============================
	// プレイヤー入力 → FlightModel
	// ============================
	Input* input = Input::GetInstance();

	// --- スロットル ---
	if (input->PushKey(DIK_LSHIFT)) {
		throttle_ += 0.8f * kDeltaTime;
	}
	if (input->PushKey(DIK_LCONTROL)) {
		throttle_ -= 0.8f * kDeltaTime;
	}
	if (throttle_ < 0.0f) throttle_ = 0.0f;
	if (throttle_ > 1.0f) throttle_ = 1.0f;
	flightModel_.SetThrottleInput(throttle_);

	// --- 自由視点カメラ (Cキー長押し) ---
	if (input->PushKey(DIK_C)) {
		if (!freeViewActive_) {
			// 押し始め: 現在のカメラ位置から球面座標を逆算して初期化
			// → 開始時にカメラがジャンプしない
			freeViewActive_ = true;
			Vector3 camOffset = Substract(cameraCurrentPos_, flightModel_.GetPosition());
			freeViewDistance_ = Length(camOffset);
			if (freeViewDistance_ < 1.0f) freeViewDistance_ = cameraDistance_;
			Vector3 dir = Normalize(camOffset);
			freeViewPitch_ = std::asin(std::clamp(dir.y, -1.0f, 1.0f));
			freeViewYaw_   = std::atan2(dir.x, dir.z);
		}
	} else {
		// Cを離したらデフォルト視点に戻る
		freeViewActive_ = false;
	}

	// --- マウスエイム ON/OFF トグル (M キー) ---
	if (input->TriggerKey(DIK_M)) {
		mouseAimEnabled_ = !mouseAimEnabled_;
		mouseAimController_.SetEnabled(mouseAimEnabled_);
		if (mouseAimEnabled_) {
			// ON に切り替えた時、目標方向を現在の機首方向にリセット & ロック
			mouseAimController_.ResetToDirection(flightModel_.GetForwardDirection());
			input->LockCursor();
		} else {
			// OFF に切り替えた時、カーソルを再表示
			input->UnlockCursor();
		}
	}

	// --- カーソルロック ON/OFF トグル (P キー) ---
	// ImGui操作用: エイムを一時停止してカーソルを解放
	if (input->TriggerKey(DIK_P)) {
		if (input->IsCursorLocked()) {
			input->UnlockCursor();
		} else {
			input->LockCursor();
		}
	}

	// --- WASD 手動操縦入力 ---
	float manualPitch = 0.0f;
	float manualRoll = 0.0f;
	float manualYaw = 0.0f;

	if (input->PushKey(DIK_W)) manualPitch -= 1.0f; // W = 機首上げ
	if (input->PushKey(DIK_S)) manualPitch += 1.0f; // S = 機首下げ
	if (input->PushKey(DIK_A)) manualRoll += 1.0f;  // A = 左ロール
	if (input->PushKey(DIK_D)) manualRoll -= 1.0f;  // D = 右ロール
	if (input->PushKey(DIK_Q)) manualYaw -= 1.0f;
	if (input->PushKey(DIK_E)) manualYaw += 1.0f;

	bool hasManualInput = (std::fabs(manualPitch) > 0.01f ||
	                       std::fabs(manualRoll) > 0.01f ||
	                       std::fabs(manualYaw) > 0.01f);

	// --- 操舵入力の決定 ---
	float finalPitch = 0.0f;
	float finalRoll = 0.0f;
	float finalYaw = 0.0f;

	if (mouseAimEnabled_ && !hasManualInput) {
		// === マウスエイムモード ===
		// カーソルロック中のみマウス入力で目標方向を更新
		// ※自由視点カメラ中はマウス操舵を抑制
		if (input->IsCursorLocked() && !freeViewActive_) {
			Input::MouseMove mouseMove = input->GetMouseMove();
			Camera* cam = CameraManager::GetInstance()->GetActiveCamera();
			if (cam) {
				const Matrix4x4& mat = cam->GetWorldMatrix();
				Vector3 camRight = { mat.m[0][0], mat.m[0][1], mat.m[0][2] };
				Vector3 camUp = { mat.m[1][0], mat.m[1][1], mat.m[1][2] };
				mouseAimController_.UpdateTargetDirection(mouseMove.lX, mouseMove.lY, camRight, camUp);
			}
		}

		// マウスエイムコントローラーがPID制御で操舵入力を生成
		mouseAimController_.CalculateSteeringInput(
			flightModel_.GetOrientation(),
			kDeltaTime,
			finalPitch, finalRoll, finalYaw
		);
	} else {
		// === 手動操縦モード（WASDオーバーライド or マウスエイムOFF）===

		// インストラクター ON/OFF (I キー)
		if (input->TriggerKey(DIK_I)) {
			flightInstructor_.ToggleEnabled();
		}

		// インストラクターによる補正（無入力時に自動水平復帰）
		flightInstructor_.ApplyCorrection(
			manualPitch, manualRoll, manualYaw,
			flightModel_.GetOrientation(),
			finalPitch, finalRoll, finalYaw
		);
	}

	flightModel_.SetControlInput(finalPitch, finalRoll, finalYaw);

	// --- フラップ / エアブレーキ ---
	flightModel_.SetFlapInput(input->PushKey(DIK_F));
	flightModel_.SetAirBrakeInput(input->PushKey(DIK_B));

	if (input->TriggerKey(DIK_V)) {
		sceneID = SCENE::RESULT;
	}

	// --- 射撃入力（マウス左クリック） ---
	if (input->PushMouse(0) && !isMissionCleared_ && !isMissionFailed_) {
		int ammoBefore = gunpod_.GetCurrentAmmo();
		gunpod_.Fire();
		// Fire() が実際に弾を消費した場合のみ弾丸エンティティを生成
		if (gunpod_.GetCurrentAmmo() < ammoBefore) {
			Vector3 firePos = flightModel_.GetPosition();
			Vector3 fireDir = flightModel_.GetForwardDirection();
			// 機首先端から発射
			firePos = Add(firePos, Multiply(5.0f, fireDir));
			bulletManager_.SpawnBullet(firePos, fireDir, 500.0f, 10.0f);
		}
	}

	// ============================
	// FlightModel 物理更新
	// ============================
	flightModel_.Update(kDeltaTime);

	// ============================
	// 戦闘システム更新
	// ============================
	gunpod_.Update(kDeltaTime);
	bulletManager_.Update(kDeltaTime);
	enemyManager_.Update();
	EffectManager::GetInstance()->Update();

	// --- 弾丸 × 敵 の衝突判定 ---
	{
		auto aliveEnemies = enemyManager_.GetAliveEnemies();
		int prevAlive = static_cast<int>(aliveEnemies.size());

		CollisionManager::CheckBulletEnemyCollisions(
			bulletManager_.GetBullets(),
			aliveEnemies,
			10.0f  // 弾丸ダメージ
		);

		// 敵が新たに破壊されたら破壊演出を発生
		for (auto* enemy : aliveEnemies) {
			if (!enemy->IsAlive()) {
				EffectManager::GetInstance()->EmitDestroyEffect(enemy->GetPosition());
			}
		}
	}

	// --- クリア判定 ---
	if (!isMissionCleared_ && enemyManager_.IsAllDestroyed()) {
		isMissionCleared_ = true;
		sceneID = SCENE::CLEAR;
	}

	// --- 地面衝突判定 ---
	if (!isMissionFailed_ && CollisionManager::CheckGroundCollision(flightModel_.GetPosition())) {
		isMissionFailed_ = true;
		sceneID = SCENE::RESULT;
	}

	// ============================
	// 描画オブジェクトに位置・姿勢を反映
	// ============================
	Vector3 pos = flightModel_.GetPosition();
	Vector3 euler = QuaternionToEuler(flightModel_.GetOrientation());

	aircraftObject_->SetTranslate(pos);
	aircraftObject_->SetRotate(euler);
	aircraftObject_->Update();

	if (skybox_) {
		skybox_->Update();
	}

	// ============================
	// 追従カメラ
	// ============================
	UpdateChaseCamera(kDeltaTime);

	// ============================
	// ImGui デバッグ UI
	// ============================
#ifdef USE_IMGUI
	ImGui::Begin("Flight Model Debug");

	ImGui::Text("=== Status ===");
	ImGui::Text("Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);

	Vector3 vel = flightModel_.GetVelocity();
	ImGui::Text("Velocity: (%.1f, %.1f, %.1f)", vel.x, vel.y, vel.z);
	ImGui::Text("Speed: %.1f m/s (%.1f km/h)", flightModel_.GetSpeed(), flightModel_.GetSpeed() * 3.6f);
	ImGui::Text("Altitude: %.1f m", pos.y);

	ImGui::Separator();
	ImGui::Text("=== Flight Data ===");
	ImGui::Text("AoA: %.1f deg", flightModel_.GetCurrentAoA() * 57.2958f);
	ImGui::Text("G-Force: %.1f G", flightModel_.GetCurrentG());
	if (flightModel_.IsStalling()) {
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "*** STALL ***");
	}
	if (flightModel_.GetBlackoutFactor() > 0.01f) {
		ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1), "Blackout: %.0f%%", flightModel_.GetBlackoutFactor() * 100.0f);
	}
	if (flightModel_.GetRedoutFactor() > 0.01f) {
		ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "Redout: %.0f%%", flightModel_.GetRedoutFactor() * 100.0f);
	}

	ImGui::Separator();
	ImGui::Text("=== Engine ===");
	ImGui::Text("Throttle: %.0f%%", throttle_ * 100.0f);
	ImGui::Text("Engine Output: %.0f%%", flightModel_.GetCurrentThrottle() * 100.0f);
	ImGui::Text("Fuel: %.1f kg", flightModel_.GetCurrentFuel());

	ImGui::Separator();
	ImGui::Text("=== Aero Devices ===");
	ImGui::Text("Flap: %.0f%%", flightModel_.GetFlapPosition() * 100.0f);
	ImGui::Text("AirBrake: %.0f%%", flightModel_.GetAirBrakePosition() * 100.0f);

	ImGui::Separator();
	ImGui::Text("=== Damage ===");
	const auto& af = flightModel_.GetAirframe();
	ImGui::Text("Engine:    %.0f%%", af.GetDamageState(DamageZone::Engine) * 100.0f);
	ImGui::Text("L.Wing:    %.0f%%", af.GetDamageState(DamageZone::LeftWing) * 100.0f);
	ImGui::Text("R.Wing:    %.0f%%", af.GetDamageState(DamageZone::RightWing) * 100.0f);
	ImGui::Text("Tail:      %.0f%%", af.GetDamageState(DamageZone::Tail) * 100.0f);
	ImGui::Text("FuelTank:  %.0f%%", af.GetDamageState(DamageZone::FuelTank) * 100.0f);

	ImGui::Separator();
	ImGui::Text("=== Mass ===");
	ImGui::Text("Total Mass: %.1f kg", flightModel_.GetTotalMass());

	ImGui::Separator();
	ImGui::Text("=== Camera ===");
	ImGui::DragFloat("Distance", &cameraDistance_, 0.5f, 5.0f, 100.0f);
	ImGui::DragFloat("Height", &cameraHeight_, 0.5f, 0.0f, 30.0f);
	ImGui::DragFloat("Pos Lag", &cameraPosLag_, 0.1f, 0.5f, 20.0f);
	ImGui::DragFloat("Look Lag", &cameraLookLag_, 0.1f, 0.5f, 30.0f);

	ImGui::Separator();
	ImGui::Text("=== Mouse Aim ===");
	ImGui::Text("Mode: %s", mouseAimEnabled_ ? "MOUSE AIM (PID)" : "MANUAL");
	if (mouseAimEnabled_) {
		// マウス感度調整スライダー
		float sens = mouseAimController_.GetSensitivity();
		if (ImGui::SliderFloat("Sensitivity", &sens, 0.01f, 1.0f, "%.3f")) {
			mouseAimController_.SetSensitivity(sens);
		}
		Vector3 tDir = mouseAimController_.GetTargetDirection();
		ImGui::Text("Target Dir: (%.2f, %.2f, %.2f)", tDir.x, tDir.y, tDir.z);

		// PIDチューニングUI
		if (ImGui::TreeNode("PID Tuning")) {
			auto pidSliders = [](const char* label, PIDController& pid) {
				if (ImGui::TreeNode(label)) {
					ImGui::SliderFloat("P", &pid.kP, 0.0f, 5.0f);
					ImGui::SliderFloat("I", &pid.kI, 0.0f, 2.0f);
					ImGui::SliderFloat("D", &pid.kD, 0.0f, 3.0f);
					ImGui::Text("Integral: %.3f", pid.integral);
					ImGui::TreePop();
				}
			};
			pidSliders("Pitch PID", mouseAimController_.GetPitchPID());
			pidSliders("Roll PID", mouseAimController_.GetRollPID());
			pidSliders("Yaw PID", mouseAimController_.GetYawPID());
			pidSliders("Bank Level PID", mouseAimController_.GetBankPID());
			ImGui::TreePop();
		}
	}
	ImGui::Text("Steering: P=%.2f R=%.2f Y=%.2f", finalPitch, finalRoll, finalYaw);

	ImGui::Separator();
	ImGui::Text("=== Controls ===");
	ImGui::Text("Mouse: Aim direction (PID)");
	ImGui::Text("W/S: Pitch | A/D: Roll | Q/E: Yaw (override)");
	ImGui::Text("LShift/LCtrl: Throttle");
	ImGui::Text("F: Flaps | B: AirBrake");
	ImGui::Text("M: Mouse Aim [%s]", mouseAimEnabled_ ? "ON" : "OFF");
	ImGui::Text("I: Instructor [%s]", flightInstructor_.IsEnabled() ? "ON" : "OFF");
	ImGui::Text("P: Cursor Lock [%s]", input->IsCursorLocked() ? "LOCKED" : "FREE");

	ImGui::End();

	// === レティクルの描画（ワールド空間→スクリーン投影）===
	if (mouseAimEnabled_) {
		Camera* cam = CameraManager::GetInstance()->GetActiveCamera();
		if (cam) {
			ImDrawList* drawList = ImGui::GetForegroundDrawList();
			const Matrix4x4& vpMat = cam->GetViewProjectionMatrix();
			float screenW = static_cast<float>(WinApp::kClientWidth);
			float screenH = static_cast<float>(WinApp::kClientHeight);
			Vector3 aircraftPos = flightModel_.GetPosition();

			// --- ワールド方向をスクリーン座標に投影するラムダ ---
			auto projectToScreen = [&](const Vector3& worldPos, float& outX, float& outY) -> bool {
				float cX = worldPos.x * vpMat.m[0][0] + worldPos.y * vpMat.m[1][0] + worldPos.z * vpMat.m[2][0] + vpMat.m[3][0];
				float cY = worldPos.x * vpMat.m[0][1] + worldPos.y * vpMat.m[1][1] + worldPos.z * vpMat.m[2][1] + vpMat.m[3][1];
				float cW = worldPos.x * vpMat.m[0][3] + worldPos.y * vpMat.m[1][3] + worldPos.z * vpMat.m[2][3] + vpMat.m[3][3];
				if (cW <= 0.001f) return false;
				outX = (cX / cW * 0.5f + 0.5f) * screenW;
				outY = (-cY / cW * 0.5f + 0.5f) * screenH;
				return true;
			};

			// --- マウスエイム目標レティクル（緑の十字線）---
			Vector3 targetDir = mouseAimController_.GetTargetDirection();
			Vector3 aimWorldPos = Add(aircraftPos, Multiply(500.0f, targetDir));
			float cx, cy;
			if (projectToScreen(aimWorldPos, cx, cy)) {
				float crossSize = 15.0f;
				float dotRadius = 3.0f;
				ImU32 reticleColor = IM_COL32(0, 255, 100, 220);
				ImU32 reticleColorDark = IM_COL32(0, 180, 70, 150);

				// 十字線
				drawList->AddLine(ImVec2(cx - crossSize, cy), ImVec2(cx - 5.0f, cy), reticleColor, 2.0f);
				drawList->AddLine(ImVec2(cx + 5.0f, cy), ImVec2(cx + crossSize, cy), reticleColor, 2.0f);
				drawList->AddLine(ImVec2(cx, cy - crossSize), ImVec2(cx, cy - 5.0f), reticleColor, 2.0f);
				drawList->AddLine(ImVec2(cx, cy + 5.0f), ImVec2(cx, cy + crossSize), reticleColor, 2.0f);
				drawList->AddCircle(ImVec2(cx, cy), dotRadius, reticleColor, 12, 2.0f);
				drawList->AddCircle(ImVec2(cx, cy), crossSize + 5.0f, reticleColorDark, 24, 1.0f);

				// --- 機首方向マーカー（白い×印）---
				Vector3 forward = flightModel_.GetForwardDirection();
				Vector3 noseWorldPos = Add(aircraftPos, Multiply(500.0f, forward));
				float nx, ny;
				if (projectToScreen(noseWorldPos, nx, ny)) {
					float noseSize = 8.0f;
					ImU32 noseColor = IM_COL32(255, 255, 255, 200);
					drawList->AddLine(ImVec2(nx - noseSize, ny - noseSize), ImVec2(nx + noseSize, ny + noseSize), noseColor, 2.0f);
					drawList->AddLine(ImVec2(nx + noseSize, ny - noseSize), ImVec2(nx - noseSize, ny + noseSize), noseColor, 2.0f);

					// 目標と機首をつなぐ薄いライン
					ImU32 lineColor = IM_COL32(255, 255, 255, 60);
					drawList->AddLine(ImVec2(cx, cy), ImVec2(nx, ny), lineColor, 1.0f);
				}
			}
		}
	}
#endif

	// === ゲーム情報UI（ImGui仮実装） ===
#ifdef USE_IMGUI
	ImGui::Begin("Mission Status");
	ImGui::Text("Enemies: %d / %d", enemyManager_.GetDestroyedCount(), enemyManager_.GetTotalCount());
	ImGui::Text("Ammo: %d / %d", gunpod_.GetCurrentAmmo(), gunpod_.GetMaxAmmo());
	ImGui::Text("Bullets Active: %u", bulletManager_.GetActiveBulletCount());
	if (isMissionCleared_) {
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "*** MISSION CLEAR ***");
	}
	if (isMissionFailed_) {
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "*** MISSION FAILED ***");
	}
	ImGui::End();
#endif

	// ============================
	// ブラックアウト ポストエフェクト（既存ビネットを利用）
	// ============================
	// 常にビネットを有効にし、blackoutFactor に応じて強度を滑らかに変化させる
	// intensity(pow指数): 0.0(透明) → 5.0(ほぼ真っ暗)
	// pow(x, 0) = 1.0 なので intensity=0 ではビネットは完全に見えない
	float blackout = flightModel_.GetBlackoutFactor();
	PostEffect* postEffect = PostEffect::GetInstance();
	postEffect->SetEffectType(PostEffectType::kVignette);
	postEffect->SetIntensity(blackout * 5.0f);
}


void StageScene::Draw() {
	// Skybox (最初に描画)
	if (skybox_) {
		SkyboxCommon::GetInstance()->SetupCommonState();
		skybox_->Draw();
	}

	// 3Dオブジェクト描画（Object3dパイプライン）
	Object3dCommon::GetInstance()->SetupCommonState();
	if (aircraftObject_) {
		aircraftObject_->Draw();
	}

	// 敵描画
	enemyManager_.Draw();

	// 地面描画（PrimitiveModelパイプライン）
	DrawGround();

	// 弾丸描画
	Camera* cam = CameraManager::GetInstance()->GetActiveCamera();
	bulletManager_.Draw(cam);

	// エフェクト描画
	EffectManager::GetInstance()->Draw(cam);
}


void StageScene::Finalize() {
	// シーン終了時にカーソルを復帰
	Input::GetInstance()->UnlockCursor();

	// エフェクトマネージャーの後片付け
	EffectManager::GetInstance()->Finalize();
}


// ===========================================================
// 地面描画（タイル式グリッド）
// ===========================================================

void StageScene::DrawGround() {
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	if (!camera) return;

	Vector3 aircraftPos = flightModel_.GetPosition();

	// --- 地面タイル ---
	// 1タイルのサイズ（メートル）
	const float tileSize = 100.0f;
	// 機体を中心にタイルをいくつ並べるか（片方向）
	const int tileRange = 10;

	// 機体位置をタイルサイズでスナップ（タイルの中心座標に丸める）
	float snapX = std::floor(aircraftPos.x / tileSize) * tileSize;
	float snapZ = std::floor(aircraftPos.z / tileSize) * tileSize;

	// タイルを敷き詰めて描画
	for (int ix = -tileRange; ix <= tileRange; ix++) {
		for (int iz = -tileRange; iz <= tileRange; iz++) {
			float cx = snapX + ix * tileSize;
			float cz = snapZ + iz * tileSize;

			// チェッカーボードパターンで色を交互に変える
			int gridX = static_cast<int>(std::floor(cx / tileSize));
			int gridZ = static_cast<int>(std::floor(cz / tileSize));
			bool isDark = ((gridX + gridZ) % 2 != 0);

			Vector4 color = isDark
				? Vector4{ 0.25f, 0.35f, 0.20f, 1.0f }   // 暗い草色
				: Vector4{ 0.35f, 0.50f, 0.28f, 1.0f };   // 明るい草色

			Vector3 scale = { tileSize, 1.0f, tileSize };
			Vector3 rotate = { 0.0f, 0.0f, 0.0f };
			Vector3 translate = { cx + tileSize * 0.5f, 0.0f, cz + tileSize * 0.5f };

			PrimitiveModel::GetInstance()->DrawPlane(
				scale, rotate, translate, color,
				groundTextureIndex_, camera
			);
		}
	}

	// --- グリッドライン（100m間隔） ---
	// 機体の周囲に十字のグリッド線を描画して距離感を出す
	const float lineExtent = tileRange * tileSize;
	const Vector4 lineColor = { 0.15f, 0.15f, 0.15f, 0.8f };

	// X方向のライン（Z軸に平行）
	for (int i = -tileRange; i <= tileRange; i++) {
		float x = snapX + i * tileSize;
		PrimitiveModel::GetInstance()->DrawLine3D(
			{ x, 0.1f, snapZ - lineExtent },
			{ x, 0.1f, snapZ + lineExtent },
			lineColor, camera
		);
	}

	// Z方向のライン（X軸に平行）
	for (int i = -tileRange; i <= tileRange; i++) {
		float z = snapZ + i * tileSize;
		PrimitiveModel::GetInstance()->DrawLine3D(
			{ snapX - lineExtent, 0.1f, z },
			{ snapX + lineExtent, 0.1f, z },
			lineColor, camera
		);
	}
}


// ===========================================================
// War Thunder風 追従カメラ
// ===========================================================
//
// ・機体の後方 + 上方にカメラの「理想位置」を計算
// ・現在のカメラ位置を Lerp で滑らかに理想位置へ追従（遅延あり）
// ・注視点も機体の少し前方を Lerp で滑らかに追従
// ・カメラの回転は LookAt で常に機体を見つめる
//
void StageScene::UpdateChaseCamera(float dt) {
	Vector3 aircraftPos = flightModel_.GetPosition();
	Vector3 forward = flightModel_.GetForwardDirection();
	Vector3 up = flightModel_.GetUpDirection();

	// ============================
	// 自由視点カメラ（Cキー長押し中）
	// ============================
	if (freeViewActive_) {
		// マウス移動量で軌道角度を更新
		Input::MouseMove mouseMove = Input::GetInstance()->GetMouseMove();
		const float sensitivity = 0.005f;
		freeViewYaw_   += static_cast<float>(mouseMove.lX) * sensitivity;
		freeViewPitch_ += static_cast<float>(mouseMove.lY) * sensitivity;

		// ピッチを制限（真上・真下を超えないように）
		const float pitchLimit = 3.141592f * 0.49f;
		if (freeViewPitch_ > pitchLimit)  freeViewPitch_ = pitchLimit;
		if (freeViewPitch_ < -pitchLimit) freeViewPitch_ = -pitchLimit;

		// 球面座標で機体中心からのオフセットを計算
		Vector3 offset;
		offset.x = freeViewDistance_ * std::cos(freeViewPitch_) * std::sin(freeViewYaw_);
		offset.y = freeViewDistance_ * std::sin(freeViewPitch_);
		offset.z = freeViewDistance_ * std::cos(freeViewPitch_) * std::cos(freeViewYaw_);

		Vector3 freeViewPos = Add(aircraftPos, offset);

		// カメラ位置・注視点を即座にセット（自由視点中は補間なし）
		cameraCurrentPos_ = freeViewPos;
		cameraLookTarget_ = aircraftPos;

		Vector3 cameraRotation = LookAtRotation(freeViewPos, aircraftPos);
		cachedCameraPitch_ = cameraRotation.x;
		cachedCameraYaw_   = cameraRotation.y;

		Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
		if (camera) {
			camera->SetTranslate(freeViewPos);
			camera->SetRotate(cameraRotation);
			Object3dCommon::GetInstance()->SetCameraPosition(freeViewPos);
		}
		return;
	}

	// ============================
	// 通常の追従カメラ
	// ============================

	// --- 理想的なカメラ位置を計算 ---
	// 機体のローカル後方（-forward）× 距離 + ローカル上方 × 高さ
	Vector3 backOffset = Multiply(-cameraDistance_, forward);
	Vector3 upOffset = Multiply(cameraHeight_, up);
	Vector3 desiredPos = Add(aircraftPos, Add(backOffset, upOffset));

	// --- 注視点は機体の少し前方 ---
	Vector3 desiredLookTarget = Add(aircraftPos, Multiply(5.0f, forward));

	// --- 滑らかに補間（Exponential Smoothing） ---
	// t = 1 - e^(-speed * dt) で、速度に依存しない滑らかな補間を実現
	float posT = 1.0f - std::exp(-cameraPosLag_ * dt);
	float lookT = 1.0f - std::exp(-cameraLookLag_ * dt);

	cameraCurrentPos_ = Lerp(cameraCurrentPos_, desiredPos, posT);
	cameraLookTarget_ = Lerp(cameraLookTarget_, desiredLookTarget, lookT);

	// --- カメラの回転を LookAt で計算 ---
	Vector3 cameraRotation = LookAtRotation(cameraCurrentPos_, cameraLookTarget_);

	// カメラの回転角をキャッシュ（マウスエイムの目標方向計算に使用）
	cachedCameraPitch_ = cameraRotation.x;
	cachedCameraYaw_ = cameraRotation.y;

	// --- カメラに反映 ---
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	if (camera) {
		camera->SetTranslate(cameraCurrentPos_);
		camera->SetRotate(cameraRotation);

		// ライティング用カメラ位置更新
		Object3dCommon::GetInstance()->SetCameraPosition(cameraCurrentPos_);
	}
}


// ===========================================================
// クォータニオン → オイラー角 変換
// ===========================================================
MyMath::Vector3 StageScene::QuaternionToEuler(const MyMath::Quaternion& q) {
	Vector3 euler{};

	// ロール (X軸回り)
	float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	euler.x = std::atan2(sinr_cosp, cosr_cosp);

	// ピッチ (Y軸回り)  — ジンバルロック対策
	float sinp = 2.0f * (q.w * q.y - q.z * q.x);
	if (std::fabs(sinp) >= 1.0f) {
		euler.y = std::copysign(3.14159265f / 2.0f, sinp);
	} else {
		euler.y = std::asin(sinp);
	}

	// ヨー (Z軸回り)
	float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	euler.z = std::atan2(siny_cosp, cosy_cosp);

	return euler;
}


// ===========================================================
// LookAt 回転の計算
// カメラ位置(from) → 注視点(to) を向くオイラー角を返す
// Camera::Update は MakeAffineMatrix(scale, rotate, translate) を使うので
// そのrotateに合致するオイラー角(XYZ順)を逆算する
// ジンバルロック対策: 真上・真下付近ではヨーを前フレームの値で維持
// ===========================================================
MyMath::Vector3 StageScene::LookAtRotation(const MyMath::Vector3& from, const MyMath::Vector3& to) const {
	Vector3 dir = Substract(to, from);
	float len = Length(dir);
	if (len < 0.0001f) {
		return { cachedCameraPitch_, cachedCameraYaw_, 0.0f };
	}

	// 正規化
	dir = Normalize(dir);

	// X軸回りの回転 (ピッチ) : -asin(y)
	// クランプしてasinの定義域を守る
	float clampedY = std::clamp(dir.y, -0.999f, 0.999f);
	float pitch = -std::asin(clampedY);

	// Y軸回りの回転 (ヨー) : atan2(x, z)
	// dir.yが±1に近い（真上・真下）とき、xとzがほぼ0でatan2が不安定になる
	// → 水平成分の大きさが閾値以下ならヨーを前フレームの値で維持
	float horizontalLen = std::sqrt(dir.x * dir.x + dir.z * dir.z);
	float yaw;
	if (horizontalLen < 0.05f) {
		// ジンバルロック領域: 前フレームのヨーを維持
		yaw = cachedCameraYaw_;
	} else {
		yaw = std::atan2(dir.x, dir.z);
	}

	// ロールは0（水平を維持）
	return { pitch, yaw, 0.0f };
}