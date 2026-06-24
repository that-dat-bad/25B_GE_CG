#include "ResultScene.h"

#ifdef USE_IMGUI
#include "../../../external/imgui/imgui.h"
#endif
ResultScene::ResultScene() {
	sceneID = SCENE::RESULT;
}

ResultScene::~ResultScene() {
}

void ResultScene::Initialize() {
}

void ResultScene::Update() {
	if (IScene::IsKeyTriggered(DIK_SPACE)) {
		sceneID = SCENE::TITLE;
	}

#ifdef USE_IMGUI
	ImGui::Begin("RESULT SCENE");
	ImGui::Text("=== RESULT SCENE ===");
	ImGui::Text("Press [SPACE] to return to Title.");
	ImGui::End();
#endif
}

void ResultScene::Draw() {
}

void ResultScene::Finalize() {
}
