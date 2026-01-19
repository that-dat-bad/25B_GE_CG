#pragma once
#include <dinput.h>

enum SCENE {
	TITLE,
	STAGE,
	CLEAR,
};

class IScene {
protected:
	static int sceneID;
	// キー入力状態
	static char keys[256];
	static char preKeys[256];

public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;
	virtual ~IScene();

	static void PollKeys();

	static bool IsKeyTriggered(int dik);

	int GetSceneID();
};
