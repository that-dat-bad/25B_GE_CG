#include "TitleScene.h"
#include <Novice.h>

void TitleScene::Initialize() {

}

void TitleScene::Update() {
	// SPACE でステージへ遷移
	if (IScene::IsKeyTriggered(DIK_SPACE)) {
		sceneID = SCENE::STAGE;
	}
}

void TitleScene::Draw() {
	Novice::ScreenPrintf(0, 0, "Title Scene");
}