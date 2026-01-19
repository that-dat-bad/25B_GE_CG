#include <dinput.h>
#include "TitleScene.h"


void TitleScene::Initialize() {

}

void TitleScene::Update() {
	// SPACE でステージへ遷移
	if (IScene::IsKeyTriggered(DIK_SPACE)) {
		sceneID = SCENE::STAGE;
	}
}

void TitleScene::Draw() {

}