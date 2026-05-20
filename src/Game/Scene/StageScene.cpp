#include "StageScene.h"
#include "CameraManager.h"
#include "Object3dCommon.h"
#include "SkyboxCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PrimitiveModel.h"
#include <cmath>

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

	// ============================
	// 描画オブジェクトの初期化
	// ============================

	// テクスチャのプリロード（モデルのマテリアルが参照するテクスチャを先に読み込む）
	TextureManager::GetInstance()->LoadTexture("assets/textures/uvChecker.png");

	// 機体モデル
	ModelManager::GetInstance()->LoadModel("Resources/planeplane.obj");
	aircraftObject_ = std::make_unique<Object3d>();
	aircraftObject_->Initialize(Object3dCommon::GetInstance());
	aircraftObject_->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	aircraftObject_->SetModel("Resources/planeplane.obj");

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
}


void StageScene::Update() {
	// ============================
	// プレイヤー入力 → FlightModel
	// ============================
	Input* input = Input::GetInstance();

	// --- スロットル ---
	static float throttle = 0.0f;
	if (input->PushKey(DIK_LSHIFT)) {
		throttle += 0.8f * kDeltaTime;
	}
	if (input->PushKey(DIK_LCONTROL)) {
		throttle -= 0.8f * kDeltaTime;
	}
	if (throttle < 0.0f) throttle = 0.0f;
	if (throttle > 1.0f) throttle = 1.0f;
	flightModel_.SetThrottleInput(throttle);

	// --- 操縦入力 ---
	float pitchInput = 0.0f;
	float rollInput = 0.0f;
	float yawInput = 0.0f;

	if (input->PushKey(DIK_W)) pitchInput -= 1.0f; // W = 機首上げ
	if (input->PushKey(DIK_S)) pitchInput += 1.0f; // S = 機首下げ
	if (input->PushKey(DIK_A)) rollInput += 1.0f;  // A = 左ロール
	if (input->PushKey(DIK_D)) rollInput -= 1.0f;  // D = 右ロール
	if (input->PushKey(DIK_Q)) yawInput -= 1.0f;
	if (input->PushKey(DIK_E)) yawInput += 1.0f;

	// --- インストラクター ON/OFF (I キー) ---
	if (input->TriggerKey(DIK_I)) {
		flightInstructor_.ToggleEnabled();
	}

	// インストラクターによる補正（無入力時に自動水平復帰）
	float correctedPitch, correctedRoll, correctedYaw;
	flightInstructor_.ApplyCorrection(
		pitchInput, rollInput, yawInput,
		flightModel_.GetOrientation(),
		correctedPitch, correctedRoll, correctedYaw
	);

	flightModel_.SetControlInput(correctedPitch, correctedRoll, correctedYaw);

	// --- フラップ / エアブレーキ ---
	flightModel_.SetFlapInput(input->PushKey(DIK_F));
	flightModel_.SetAirBrakeInput(input->PushKey(DIK_B));

	if (input->TriggerKey(DIK_V)) {
		sceneID = SCENE::RESULT;
	}

	// ============================
	// FlightModel 物理更新
	// ============================
	flightModel_.Update(kDeltaTime);

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
	ImGui::Text("Throttle: %.0f%%", throttle * 100.0f);
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
	ImGui::Text("=== Controls ===");
	ImGui::Text("W/S: Pitch | A/D: Roll | Q/E: Yaw");
	ImGui::Text("LShift/LCtrl: Throttle");
	ImGui::Text("F: Flaps | B: AirBrake");
	ImGui::Text("I: Instructor [%s]", flightInstructor_.IsEnabled() ? "ON" : "OFF");

	ImGui::End();
#endif
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

	// 地面描画（PrimitiveModelパイプライン — Object3dの後に描画）
	DrawGround();
}


void StageScene::Finalize() {
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
// ===========================================================
MyMath::Vector3 StageScene::LookAtRotation(const MyMath::Vector3& from, const MyMath::Vector3& to) {
	Vector3 dir = Substract(to, from);
	float len = Length(dir);
	if (len < 0.0001f) {
		return { 0.0f, 0.0f, 0.0f };
	}

	// 正規化
	dir = Normalize(dir);

	// Y軸回りの回転 (ヨー) : atan2(x, z)
	float yaw = std::atan2(dir.x, dir.z);

	// X軸回りの回転 (ピッチ) : -asin(y)  — 上を向くときは負のピッチ
	float pitch = -std::asin(dir.y);

	// ロールは0（水平を維持）
	return { pitch, yaw, 0.0f };
}