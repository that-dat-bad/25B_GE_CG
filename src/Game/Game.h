#pragma once
#include <memory>
#include <vector>
#include <string>
#include "../engine/base/winApp.h"
#include "../engine/Graphics/DirectXCommon.h"
#include "../engine/io/Input.h"
#include "../engine/Audio/AudioManager.h"
#include "../engine/Graphics/SrvManager.h"
#include "../engine/Debug/ImguiManager.h"
#include "../engine/Graphics/TextureManager.h"
#include "../engine/Graphics/ModelManager.h"
#include "../engine/Graphics/CameraManager.h"
#include "../engine/Graphics/SpriteCommon.h"
#include "../engine/Graphics/Object3dCommon.h"
#include "../engine/Graphics/Object3d.h"
#include "../engine/Graphics/ParticleManager.h"

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
	Object3d* sphereObject = nullptr;
	SoundData alarmSound;

	// ライト・設定用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> lightingSettingsResource;
	LightingSettings* lightingSettingsData = nullptr;

	// ゲーム終了フラグ
	bool endRequest_ = false;
};