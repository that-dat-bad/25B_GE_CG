#include "IScene.h"
#include"Input.h"
#include <cstring>

int IScene::currentScene = SCENE::TITLE;

IScene::~IScene() = default;

bool IScene::IsKeyTriggered(BYTE keyNumber) {
	return Input::GetInstance()->TriggerKey(keyNumber);
}

bool IScene::IsKeyPressed(BYTE keyNumber) {
	return Input::GetInstance()->PushKey(keyNumber);
}

int IScene::GetCurrentScene() {
	return currentScene;
}

