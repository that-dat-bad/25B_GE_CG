#pragma once
#include "IScene.h"
#include "Camera.h"
#include "Input.h"
#include "math/MyMath.h"

class Sprite;

class ClearScene : public IScene {
public:
	~ClearScene() override;
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;
	void Finalize() override;

	static bool isWin;
	static int finalScore;

private:
	Camera camera_;
	Input* input_ = nullptr;
	
	Sprite* winSprite_ = nullptr;
	Sprite* loseSprite_ = nullptr;
	Sprite* numberSprites_[10] = { nullptr };
};
