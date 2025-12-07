#pragma once
#include "Fade.h"
#include "BackGround.h"

class ClearScene {
public:
	~ClearScene();
	void Initialize();
	void Update();
	void Draw();
	bool IsFinished() const { return isFinished_; }

private:
	void UpdateFadeIn();
	void UpdateMain();
	void UpdateFadeOut();

	enum class Phase { kFadeIn, kMain, kFadeOut };
	Phase phase_ = Phase::kMain;

	Fade* fade_ = nullptr;
	BackGround* backGround_ = nullptr;
	bool isFinished_ = false;
	float duration_ = 0.5f;
};