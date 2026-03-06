#pragma once
#include "IScene.h"
#include "Camera.h"
#include "Input.h"
#include "math/MyMath.h"
#include <memory>

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
	
	std::unique_ptr<Sprite> winSprite_;
	std::unique_ptr<Sprite> loseSprite_;
	std::unique_ptr<Sprite> numberSprites_[10];
};
