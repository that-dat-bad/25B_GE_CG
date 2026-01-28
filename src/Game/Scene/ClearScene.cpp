#include "ClearScene.h"
#include "ImguiManager.h"
#include "Object3dCommon.h"

using namespace MyMath;

bool ClearScene::isWin = false;
int ClearScene::finalScore = 0;

ClearScene::~ClearScene() { }

void ClearScene::Initialize() {
	// camera_.Initialize();
	input_ = Input::GetInstance();

	camera_.SetTranslate({ 0.0f, 0.0f, -10.0f });
}

void ClearScene::Finalize() {
}

std::optional<SceneID> ClearScene::Update() {
	if (input_->TriggerKey(DIK_SPACE)) {
		return SceneID::kTitle;
	}
	return std::nullopt;
}

void ClearScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Object3dCommon::GetInstance()->SetupCommonState();

#ifdef USE_IMGUI

	ImGui::Begin("RESULT");

	if (isWin) {
		ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "MISSION ACCOMPLISHED");
	} else {
		ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "MISSION FAILED");
	}

	ImGui::Separator();

	ImGui::Text("FINAL SCORE: %d", finalScore);

	ImGui::Text(" ");
	ImGui::TextColored({ 0.5f, 0.5f, 0.5f, 1.0f }, "Push SPACE to Return");

	ImGui::End();
#endif // DEBUG
}
