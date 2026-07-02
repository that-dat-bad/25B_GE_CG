#pragma once
#include <memory>
#include <vector>
#include <string>
#include "WinApp.h"
#include "../../engine/Graphics/System/DirectXCommon.h"
#include "Input.h"
#include "AudioManager.h"
#include "../../engine/Graphics/System/SrvManager.h"
#include "ImguiManager.h"
#include "../../engine/Graphics/System/TextureManager.h"
#include "../../engine/Graphics/Model/ModelManager.h"
#include "../../engine/Graphics/Camera/CameraManager.h"
#include "../../engine/Graphics/Sprite/SpriteCommon.h"
#include "../../engine/Graphics/Model/Object3dCommon.h"
#include "../../engine/Graphics/Model/Object3d.h"
#include "../../engine/Graphics/Model/SkyboxCommon.h"
#include "../../engine/Graphics/Particle/ParticleManager.h"
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




	// ゲーム終了フラグ
	bool endRequest_ = false;

	//シーン管理
	std::unique_ptr<SceneManager> sceneManager = nullptr;
};