#pragma once
#include "IScene.h"
#include "Camera.h"
#include "Input.h"
#include "math/MyMath.h"

class ClearScene : public IScene {
public:
	~ClearScene() override;
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;
	void Finalize() override;

	// 勝敗フラグ
	static bool isWin;
	static int finalScore;

private:
	Camera camera_;
	Input* input_ = nullptr;
};