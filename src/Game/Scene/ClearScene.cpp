#include "ResultScene.h"
#include "KamataEngine.h"

using namespace KamataEngine;

// 静的変数の定義
bool ResultScene::isWin = false;
int ResultScene::finalScore = 0;

ResultScene::~ResultScene() { delete model_; }

void ResultScene::Initialize() {
	camera_.Initialize();
	worldTransform_.Initialize();
	input_ = Input::GetInstance();

	// 画面中央に配置
	camera_.translation_.z = -10.0f;
}

std::optional<SceneID> ResultScene::Update() {
	// スペースキーでタイトルに戻る
	if (input_->TriggerKey(DIK_SPACE)) {
		return SceneID::kTitle;
	}
	return std::nullopt;
}

void ResultScene::Draw() {
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();
	KamataEngine::Model::PreDraw(dxCommon->GetCommandList());
	// モデルがあれば描画
	KamataEngine::Model::PostDraw();

#ifdef _DEBUG

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