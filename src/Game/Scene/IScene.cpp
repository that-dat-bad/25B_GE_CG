#include "IScene.h"
#include <Novice.h>
#include <cstring>

int IScene::sceneID = SCENE::TITLE;
char IScene::keys[256] = {0};
char IScene::preKeys[256] = {0};

IScene::~IScene() = default;

void IScene::PollKeys() {
	// 前フレームの状態を保存してから現在のキー状態を取得
	std::memcpy(preKeys, keys, sizeof(keys));
	Novice::GetHitKeyStateAll(keys);
}

bool IScene::IsKeyTriggered(int dik) {
	return preKeys[dik] == 0 && keys[dik] != 0;
}

int IScene::GetSceneID() {
	return sceneID;
}

