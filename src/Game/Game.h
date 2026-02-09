#pragma once
#include <memory>
#include <vector>
#include <string>
#include "WinApp.h"
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




class Game {
public:
	void Initialize();
	void Finalize();
	void Update();
	void Draw();
	void Run();
	bool IsEndRequest() const { return endRequest_; }

private:
	std::unique_ptr<WinApp> winApp = nullptr;
	std::unique_ptr<SrvManager> srvManager = nullptr;
	std::unique_ptr<ImGuiManager> imguiManager = nullptr;
	std::unique_ptr<AudioManager> audioManager = nullptr;

	std::unique_ptr<SpriteCommon> spriteCommon = nullptr;

	SoundData alarmSound;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> lightingSettingsResource;
	LightingSettings* lightingSettingsData = nullptr;

	bool endRequest_ = false;

	std::unique_ptr<SceneManager> sceneManager = nullptr;
};
