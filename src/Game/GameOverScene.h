#pragma once
#include "Fade.h"
#include "BackGround.h"
#include "AudioManager.h"

class GameOverScene {
public:
	~GameOverScene();
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

	// BGM
	SoundData soundBgm_;
	SoundData soundSe_;
	IXAudio2SourceVoice* pBgmVoice_ = nullptr;
};