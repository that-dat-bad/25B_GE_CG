#pragma once
#include "IScene.h"
#include <optional>
#include "math/MyMath.h"
#include "Object3d.h"
#include "Sprite.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "Input.h"
#include "ModelManager.h"

// 前方宣言
class TitleLogo;

class TitleScene : public IScene {
public:
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;
	void Finalize() override;
	~TitleScene() noexcept override;

private:
	// --- フェード用 ---
	std::unique_ptr<Sprite> fadeSprite_ = nullptr;
	uint32_t fadeTextureHandle_ = 0;

	Camera camera_;
	DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	// --- 入力 ---
	Input* input_ = nullptr;

	// --- フェーズ管理 ---
	std::optional<SceneID> UpdateFadeIn();
	std::optional<SceneID> UpdateMain();
	std::optional<SceneID> UpdateFadeOut();

	// --- タイトルロゴ (3D) ---
	TitleLogo* logo_ = nullptr;
	MyMath::Vector3 logoPosition_ = { 0.0f, 0.0f, -45.0f };

	// --- 背景 (天球) ---
	// モデルデータとオブジェクトを分ける
	Model* skydomeModelResource_ = nullptr;
	std::unique_ptr<Object3d> skydomeObject_ = nullptr;
	
	// --- 誘導表示 (スプライト) ---
	std::unique_ptr<Sprite> pressSpaceSprite_ = nullptr; // スプライト本体
	uint32_t pressSpaceTexture_ = 0;                   // テクスチャハンドル
	float blinkTimer_ = 0.0f;                          // 点滅用タイマー
};