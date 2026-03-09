#pragma once
#include "IScene.h"
#include <memory>
#include "Object3d.h"
#include "Camera.h"

class TitleScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	Camera camera_;

	// Ruruko モデル表示用
	std::unique_ptr<Object3d> rurukoObject_ = nullptr;
};