#pragma once
#include "IScene.h"
#include <optional>
#include "math/MyMath.h"
#include "Object3d.h"
#include "Sprite.h"
#include "Camera.h"
class TitleLogo;
class Input;
class DebugCamera;

class TitleScene : public IScene {
public:
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;
	void Finalize() override;
	~TitleScene() noexcept override;

private:
	std::unique_ptr<Sprite> fadeSprite_ = nullptr;
	uint32_t fadeTextureHandle_ = 0;

	Camera camera_;
	DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	Input* input_ = nullptr;

	std::optional<SceneID> UpdateFadeIn();
	std::optional<SceneID> UpdateMain();
	std::optional<SceneID> UpdateFadeOut();

	TitleLogo* logo_ = nullptr;
	MyMath::Vector3 logoPosition_ = { 0.0f, 0.0f, -45.0f };

	Model* skydomeModelResource_ = nullptr;
	std::unique_ptr<Object3d> skydomeObject_ = nullptr;
	
	std::unique_ptr<Sprite> pressSpaceSprite_ = nullptr; 
	uint32_t pressSpaceTexture_ = 0;                   
	float blinkTimer_ = 0.0f;                          
};
