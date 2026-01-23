#include "CameraManager.h"
#include "StageScene.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
#include "../../../external/imgui/imgui.h"
#include "../../engine/io/Input.h"
#include "../../engine/base/Math/MyMath.h"

#include <cmath>
#include <ModelManager.h>

void StageScene::Initialize() {
	sceneID = SCENE::STAGE;

	// --- 3Dオブジェクト ---
	// --- 3Dオブジェクト ---
	// --- 3Dオブジェクト ---
	sphereObject = std::make_unique<Object3d>();
	sphereObject->Initialize(Object3dCommon::GetInstance());
	sphereObject->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	ModelManager::GetInstance()->LoadModel("models/sphere.obj");
	ModelManager::GetInstance()->LoadModel("models/plane.gltf");
	ModelManager::GetInstance()->LoadModel("models/plane.obj");
	sphereObject->SetModel("models/sphere.obj");

	terrainObject = std::make_unique<Object3d>();
	terrainObject->Initialize(Object3dCommon::GetInstance());
	terrainObject->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	ModelManager::GetInstance()->LoadModel("models/terrain.obj");
	terrainObject->SetModel("models/terrain.obj");
	
	// Position terrain slightly lower
	terrainObject->SetTranslate({ 0.0f, -1.0f, 0.0f });



	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 0.0f, 0.0f, -10.0f });
	CameraManager::GetInstance()->Update();
}

void StageScene::Update() {
	// Camera Controls
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	if (camera) {
		Vector3 rotate = camera->GetRotate();
		Vector3 translate = camera->GetTranslate();

		const float kRotSpeed = 0.02f;
		const float kMoveSpeed = 0.1f;

		// Rotation
		if (Input::GetInstance()->PushKey(DIK_UP)) rotate.x -= kRotSpeed;
		if (Input::GetInstance()->PushKey(DIK_DOWN)) rotate.x += kRotSpeed;
		if (Input::GetInstance()->PushKey(DIK_LEFT)) rotate.y -= kRotSpeed;
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) rotate.y += kRotSpeed;

		// Mouse Rotation
		// 0: Left, 1: Right, 2: Middle
		if (Input::GetInstance()->PushMouse(1)) {
			Input::MouseMove mouseMove = Input::GetInstance()->GetMouseMove();
			const float kMouseSensitivity = 0.003f;
			rotate.y += mouseMove.lX * kMouseSensitivity;
			rotate.x += mouseMove.lY * kMouseSensitivity;
		}

		// Movement (Relative to Y rotation)
		Matrix4x4 matRot = MakeRotateYMatrix(rotate.y);
		Vector3 moveDir = { 0.0f, 0.0f, 0.0f };

		if (Input::GetInstance()->PushKey(DIK_W)) moveDir.z += kMoveSpeed;
		if (Input::GetInstance()->PushKey(DIK_S)) moveDir.z -= kMoveSpeed;
		if (Input::GetInstance()->PushKey(DIK_A)) moveDir.x -= kMoveSpeed;
		if (Input::GetInstance()->PushKey(DIK_D)) moveDir.x += kMoveSpeed;
		if (Input::GetInstance()->PushKey(DIK_E)) moveDir.y += kMoveSpeed;
		if (Input::GetInstance()->PushKey(DIK_Q)) moveDir.y -= kMoveSpeed;

		// Transform moveDir by rotation matrix
		moveDir = TransformNormal(moveDir, matRot);

		translate = Add(translate, moveDir);

		if (Input::GetInstance()->TriggerKey(DIK_R)) {
			rotate = { 0.0f, 0.0f, 0.0f };
			translate = { 0.0f, 0.0f, -10.0f };
		}

		camera->SetRotate(rotate);
		camera->SetTranslate(translate);
	}

	sphereObject->Update();
	terrainObject->Update();



#ifdef USE_IMGUI
	ImGui::Begin("Lighting Settings");
	if (ImGui::CollapsingHeader("Controls Help")) {
		ImGui::Text("WASD : Move Horizontal");
		ImGui::Text("Q / E : Move Vertical");
		ImGui::Text("Arrows : Rotate Camera");
		ImGui::Text("R-Click + Drag : Rotate Camera");
		ImGui::Text("R : Reset Camera");
		ImGui::Separator();
	}

	static int currentShading = 0;
	const char* shadingItems[] = { "Lambert", "Half-Lambert" };
	if (ImGui::Combo("Shading Model", &currentShading, shadingItems, IM_ARRAYSIZE(shadingItems))) {
		Object3dCommon::GetInstance()->SetShadingModel(currentShading);
	}

	static int currentSpecular = 0;
	const char* specularItems[] = { "None", "Phong", "Blinn-Phong" };
	if (ImGui::Combo("Specular Model", &currentSpecular, specularItems, IM_ARRAYSIZE(specularItems))) {
		Object3dCommon::GetInstance()->SetSpecularModel(currentSpecular);
	}

	static bool enableDirectional = false;
	static bool enablePoint = false;
	static bool enableSpot = false;
	
	bool changed = false;
	changed |= ImGui::Checkbox("Enable Directional Light", &enableDirectional);
	changed |= ImGui::Checkbox("Enable Point Light", &enablePoint);
	changed |= ImGui::Checkbox("Enable Spot Light", &enableSpot);

	if (changed) {
		int lightType = 0;
		if (enableDirectional) lightType |= 1;
		if (enablePoint) lightType |= 2;
		if (enableSpot) lightType |= 4;
		
		Object3dCommon::GetInstance()->SetLightType(lightType);
	}

	if (ImGui::CollapsingHeader("Directional Light")) {
		DirectionalLight* lightData = Object3dCommon::GetInstance()->GetDirectionalLightData();
		ImGui::DragFloat3("Direction", &lightData->direction.x, 0.01f);
		lightData->direction = Normalize(lightData->direction);
		ImGui::ColorEdit4("Color", &lightData->color.x);
		ImGui::DragFloat("Intensity", &lightData->intensity, 0.01f);
	}

	if (ImGui::CollapsingHeader("Point Light")) {
		PointLight* pointData = Object3dCommon::GetInstance()->GetPointLightData();
		ImGui::DragFloat3("Position", &pointData->position.x, 0.1f);
		ImGui::ColorEdit4("Color", &pointData->color.x);
		ImGui::DragFloat("Intensity", &pointData->intensity, 0.01f);
		ImGui::DragFloat("Radius", &pointData->radius, 0.1f);
		ImGui::DragFloat("Decay", &pointData->decay, 0.01f);
	}

	if (ImGui::CollapsingHeader("Spot Light")) {
		SpotLight* spotData = Object3dCommon::GetInstance()->GetSpotLightData();
		ImGui::DragFloat3("Position", &spotData->position.x, 0.1f);
		ImGui::DragFloat3("Direction", &spotData->direction.x, 0.01f);
		spotData->direction = Normalize(spotData->direction);
		ImGui::ColorEdit4("Color", &spotData->color.x);
		ImGui::DragFloat("Intensity", &spotData->intensity, 0.01f);
		ImGui::DragFloat("Distance", &spotData->distance, 0.1f);
		ImGui::DragFloat("Decay", &spotData->decay, 0.01f);
		ImGui::DragFloat("Cos Angle", &spotData->cosAngle, 0.01f);
		ImGui::DragFloat("Cos Falloff Start", &spotData->cosFalloffStart, 0.01f);
	}
	
	if (sphereObject) {
		Vector3 scale = sphereObject->GetScale();
		if (ImGui::DragFloat3("Sphere Scale", &scale.x, 0.01f)) {
			sphereObject->SetScale(scale);
		}
	}

	if (sphereObject && sphereObject->GetModel()) {
		float shininess = sphereObject->GetModel()->GetShininess();
		if (ImGui::DragFloat("Shininess", &shininess, 1.0f, 1.0f, 256.0f)) {
			sphereObject->GetModel()->SetShininess(shininess);
		}
	}

	if (ImGui::CollapsingHeader("Model Selection")) {
		static int currentModelIndex = 0;
		if (ImGui::RadioButton("Sphere", &currentModelIndex, 0)) {
			ModelManager::GetInstance()->LoadModel("models/sphere.obj");
			sphereObject->SetModel("models/sphere.obj");
			sphereObject->SetRotate({ 0.0f, 0.0f, 0.0f });
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Plane", &currentModelIndex, 1)) {
			ModelManager::GetInstance()->LoadModel("models/plane.gltf");
			sphereObject->SetModel("models/plane.gltf");
			sphereObject->SetRotate({ 0.0f, 3.141592f, 0.0f });
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("PlaneOBJ", &currentModelIndex, 2)) {
			ModelManager::GetInstance()->LoadModel("models/plane.obj");
			sphereObject->SetModel("models/plane.obj");
			sphereObject->SetRotate({ 0.0f, 3.141592f, 0.0f });
		}
		Vector3 modelPos = sphereObject->GetTranslate();
		if (ImGui::DragFloat3("Model Translate", &modelPos.x, 0.01f)) {
			sphereObject->SetTranslate(modelPos);
		}
	}

	ImGui::End();
#endif
	
	if (camera) {
		Object3dCommon::GetInstance()->SetCameraPosition(camera->GetTranslate());
	}
}

void StageScene::Draw() {
	// 1. 3D描画
	Object3dCommon::GetInstance()->SetupCommonState();
	sphereObject->Draw();
	terrainObject->Draw();


}

void StageScene::Finalize() {
}