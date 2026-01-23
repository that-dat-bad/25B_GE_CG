#pragma once
#include "IScene.h"
#include <memory> 

class SceneManager {
private:
	std::unique_ptr<IScene> currentScene = nullptr;
	int currentSceneID; // 現在管理しているシーンIDを保持

public:
	SceneManager();
	~SceneManager();
	void Update();
	void Draw();
};