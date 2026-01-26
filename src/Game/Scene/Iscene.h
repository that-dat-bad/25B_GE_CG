#pragma once
#include"Input.h"

enum SCENE {
	TITLE,
	GAME,
	CLEAR,
};

class IScene {
public:

	enum ScenePhase {
		kFadeIn,
		kMain,
		kFadeOut,
	};

protected:

	static int currentScene;
	ScenePhase phase_ = ScenePhase::kFadeIn;
	int fadeTimer_ = 30;
	const int kFadeDuration_ = 30;

public:

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;
	virtual ~IScene() {};

	static bool IsKeyTriggered(BYTE keyNumber);
	static bool IsKeyPressed(BYTE keyNumber);

	int GetCurrentScene();
};
