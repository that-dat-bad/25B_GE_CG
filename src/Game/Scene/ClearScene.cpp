#include "ClearScene.h"


void ClearScene::Initialize() {
}

void ClearScene::Update() {

	if (IScene::IsKeyTriggered(DIK_SPACE)) {
		sceneID = SCENE::TITLE;
	}
}

void ClearScene::Draw() {

}

void ClearScene::Finalize() {
}

ClearScene::ClearScene() {
	sceneID = SCENE::CLEAR;
}

ClearScene::~ClearScene() {
}

