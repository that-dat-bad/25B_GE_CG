#include "ClearScene.h"
#include <Novice.h>
#include <iostream>

void ClearScene::Initialize() {
	std::cout << "Clear Initialized" << std::endl;
}

void ClearScene::Update() {

	if (IScene::IsKeyTriggered(DIK_SPACE)) {
		sceneID = SCENE::TITLE;
	}
}

void ClearScene::Draw() {
	Novice::ScreenPrintf(0, 0, "Clear Scene");
}

ClearScene::ClearScene() {
	sceneID = SCENE::CLEAR;
}

ClearScene::~ClearScene() {
}

