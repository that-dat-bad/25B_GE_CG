#pragma once
#include "IScene.h"
#include <memory> 

class SceneManager {
private:
	std::unique_ptr<IScene> currentScene = nullptr;
	int currentScene;

public:
	SceneManager();
	~SceneManager();
	void Update();
	void Draw();
};