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
	// ゲーム終了フラグのチェック
	bool IsEndRequest() const { return endRequest_; }

private:
	std::unique_ptr<WinApp> winApp = nullptr;
	std::unique_ptr<SrvManager> srvManager = nullptr;
	std::unique_ptr<ImGuiManager> imguiManager = nullptr;
	std::unique_ptr<AudioManager> audioManager = nullptr;

	std::unique_ptr<SpriteCommon> spriteCommon = nullptr;

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
	std::unique_ptr<SceneManager> sceneManager = nullptr;
};