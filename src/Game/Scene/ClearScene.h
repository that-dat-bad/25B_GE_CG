#pragma once
#include "IScene.h"
#include "KamataEngine.h"

class ResultScene : public IScene {
public:
	~ResultScene() override;
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;

	// 勝敗フラグ
	static bool isWin;
	static int finalScore;

private:
	KamataEngine::Camera camera_;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
};