#pragma once
#include "IScene.h"
#include <optional>
#include "Camera.h"
#include "Object3d.h"
#include "Sprite.h"
#include "Title/TitleLogo.h"

class TitleScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
	~TitleScene() noexcept override;

private:
	// --- フェード用 ---
	uint32_t textureHandle_ = 0;
	std::unique_ptr<Sprite> fadeSprite_ = nullptr;
	uint32_t fadeTextureHandle_ = 0;


	Camera camera_;

	// --- 入力 ---
	Input* input_ = nullptr;

	// --- フェーズ管理 ---
	void UpdateFadeIn();
	void UpdateMain();
	void UpdateFadeOut();

	// --- タイトルロゴ (3D) ---
	TitleLogo* logo_ = nullptr;
	MyMath::Vector3 logoPosition_ = { 0.0f, 0.0f, -45.0f };

	// --- 背景 (天球) ---
	std::unique_ptr<Object3d> skydomeModel_ = nullptr;

	// --- 誘導表示 (スプライト) ---
	std::unique_ptr<Sprite> pressSpaceSprite_ = nullptr; // スプライト本体
	uint32_t pressSpaceTexture_ = 0;                   // テクスチャハンドル
	float blinkTimer_ = 0.0f;                          // 点滅用タイマー
};