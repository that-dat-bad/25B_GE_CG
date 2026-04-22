#pragma once
#include "IScene.h"

/// @brief ゲーム本編のステージシーン
class StageScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};