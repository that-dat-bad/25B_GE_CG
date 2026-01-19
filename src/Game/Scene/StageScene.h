#pragma once
#include "IScene.h"
#include <vector>
#include "Object3d.h"


class StageScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
private:
	Object3d* sphereObject = nullptr;
};