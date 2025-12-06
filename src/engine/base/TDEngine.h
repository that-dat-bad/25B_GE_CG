#pragma once
#include <Windows.h>
#include <string>
#include <cstdint>


#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "AudioManager.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Model.h"
#include "WorldTransform.h"
#include "Math/MyMath.h"
#include "Camera.h"
namespace TDEngine {

	Input* GetInput();
	Object3dCommon* GetObject3dCommon();
	SpriteCommon* GetSpriteCommon();
	AudioManager* GetAudioManager();

	void Initialize(const std::wstring& title, int width = 1280, int height = 720);

	// エンジン更新処理 (ウィンドウメッセージ処理など)
	bool Update();

	// 終了処理
	void Finalize();
}