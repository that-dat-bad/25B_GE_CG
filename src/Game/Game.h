#pragma once
#include <memory>
#include <vector>
#include <string>
#include "winApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "AudioManager.h"
#include "SrvManager.h"
#include "ImguiManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "CameraManager.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ParticleManager.h"
#include "SceneManager.h"


struct DirectionalLight {
    Vector4 color;
    Vector3 direction;
    float intensity;
};

struct LightingSettings {
    int32_t lightingModel; // 0: Lambert, 1: Half-Lambert
    float padding[3];
};

class Game {
public:
	void Initialize();
	void Finalize();
	void Update();
	void Draw();
	void Run();
	// ゲーム終了フラグのチェック
	bool IsEndRequest() const { return endRequest_; }

private:
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	Input* input = nullptr;
	SrvManager* srvManager = nullptr;
	ImGuiManager* imguiManager = nullptr;
	AudioManager* audioManager = nullptr;

	SpriteCommon* spriteCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;

	// ゲームオブジェクト
	SoundData alarmSound;

	// ライト・設定用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> lightingSettingsResource;
	LightingSettings* lightingSettingsData = nullptr;

	// ゲーム終了フラグ
	bool endRequest_ = false;

	//シーン管理
	SceneManager* sceneManager = nullptr;
};