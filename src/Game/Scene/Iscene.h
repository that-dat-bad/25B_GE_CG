#pragma once

enum SCENE {
	TITLE,
	STAGE,
	CLEAR,
};

class IScene {
protected:
	static int sceneID;
	// キー入力状態（フレームごとに更新）
	static char keys[256];
	static char preKeys[256];

public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual ~IScene();

	// フレームごとのキー状態更新（SceneManager::Update の先頭で1回呼ぶ）
	static void PollKeys();

	// 指定キーが前フレーム:0 -> 今フレーム:押下 の遷移（トリガー）を判定
	static bool IsKeyTriggered(int dik);

	int GetSceneID();
};
