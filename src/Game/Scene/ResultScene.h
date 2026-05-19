#pragma once
#include "IScene.h"

class ResultScene : public IScene {
public:
	ResultScene();
	~ResultScene();
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};
