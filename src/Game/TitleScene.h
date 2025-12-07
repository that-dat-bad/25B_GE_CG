#pragma once
#include "BackGround.h"
#include "TitleLogo.h"
#include "Fade.h"

class TitleScene {
public:
	enum class Select { kNone, kTutorial, kGame };
	Select select_ = Select::kNone;

	enum class Phase { kFadeIn, kMain, kFadeOut };
	Phase phase_ = Phase::kMain;

	~TitleScene();
	void Initialize();
	void Update();
	void Draw();

	Select GetSelect() const { return select_; }
	bool IsFinished() const { return isFinished_; }

private:
	void UpdateFadeIn();
	void UpdateMain();
	void UpdateFadeOut();
	void UpdateSelect();

	Fade* fade_ = nullptr;
	float duration_ = 0.5f;
	bool isFinished_ = false;

	BackGround* backGround_ = nullptr;
	TitleLogo* logo_ = nullptr;
};