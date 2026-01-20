#pragma once
#include "IScene.h"

class SceneManager {
private:
	IScene* currentScene = nullptr;
	int currentSceneID; // 現在管理しているシーンIDを保持

public:
	SceneManager();
	~SceneManager();
	void Update();
	void Draw();
};