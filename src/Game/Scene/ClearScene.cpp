#include "ClearScene.h"
#include "ImguiManager.h"

using namespace MyMath;

// 静的変数の定義
bool ClearScene::isWin = false;
int ClearScene::finalScore = 0;

ClearScene::~ClearScene() { }

void ClearScene::Initialize() {
	camera_.Initialize();
	input_ = Input::GetInstance();

	// 画面中央に配置
	// 画面中央に配置
	camera_.translation_.z = -10.0f;
}

void ClearScene::Finalize() {
}

std::optional<SceneID> ClearScene::Update() {
	// スペースキーでタイトルに戻る
	if (input_->TriggerKey(DIK_SPACE)) {
		return SceneID::kTitle;
	}
	return std::nullopt;
}

void ClearScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	// モデルがあれば描画
	Model::PostDraw();

#ifdef USE_IMGUI

	// --- 結果表示 (ImGui) ---
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