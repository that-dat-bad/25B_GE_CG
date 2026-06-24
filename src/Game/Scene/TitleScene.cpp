#define DIRECTINPUT_VERSION     0x0800
#include <dinput.h>
#include "TitleScene.h"

#ifdef USE_IMGUI
#include "../../../external/imgui/imgui.h"
#endif

void TitleScene::Initialize() {

}

void TitleScene::Update() {
	if (IScene::IsKeyTriggered(DIK_SPACE)) {
		sceneID = SCENE::STAGE;
	}

#ifdef USE_IMGUI
	ImGui::Begin("TITLE SCENE");
	ImGui::Text("=== TITLE SCENE ===");
	ImGui::Text("Press [SPACE] to start.");
	ImGui::End();
#endif
}

void TitleScene::Draw() {

}

void TitleScene::Finalize() {
}
