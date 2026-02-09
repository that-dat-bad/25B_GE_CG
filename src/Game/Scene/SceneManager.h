#pragma once
#include "IScene.h"
#include <memory> 

class SceneManager {
private:
	std::unique_ptr<IScene> currentScene_ = nullptr;
	SceneID currentSceneID_ = SceneID::kTitle;

public:
	SceneManager();
	~SceneManager();
	void Update();
	void Draw();
};
