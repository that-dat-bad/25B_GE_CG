#include "TDEngine.h"

#include "StringUtility.h"
#include "Logger.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "CameraManager.h"
#include "ParticleManager.h"
#include "AudioManager.h"

using namespace TDEngine;

static WinApp* sWinApp = nullptr;
static Input* sInput = nullptr;
static DirectXCommon* sDxCommon = nullptr;
static SrvManager* sSrvManager = nullptr;
static ImGuiManager* sImGuiManager = nullptr;
static AudioManager* sAudioManager = nullptr;
static SpriteCommon* sSpriteCommon = nullptr;
static Object3dCommon* sObject3dCommon = nullptr;

void TDEngine::Initialize(const std::wstring& title, int width, int height) {
	// --- 基盤システムの初期化 ---
	sWinApp = new WinApp();

	sWinApp->Initialize(title);


	sInput = new Input();
	sInput->Initialize(sWinApp->GetInstance(), sWinApp->GetHwnd());

	sDxCommon = DirectXCommon::GetInstance();
	sDxCommon->Initialize(sWinApp);

	sSrvManager = new SrvManager();
	sSrvManager->Initialize(sDxCommon);

	// ImGuiManagerもシングルトン化している想定
	sImGuiManager = ImGuiManager::GetInstance();
	sImGuiManager->Initialize(sWinApp, sDxCommon, sSrvManager);

	sAudioManager = new AudioManager();
	sAudioManager->Initialize();

	// コマンドリストリセット
	HRESULT hr = sDxCommon->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = sDxCommon->GetCommandList()->Reset(sDxCommon->GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(hr));

	// --- マネージャ初期化 ---
	TextureManager::GetInstance()->Initialize(sDxCommon, sSrvManager);
	ModelManager::GetInstance()->Initialize(sDxCommon);
	ParticleManager::GetInstance()->Initialize(sDxCommon, sSrvManager);

	// --- 共通設定初期化 ---
	sSpriteCommon = new SpriteCommon();
	sSpriteCommon->Initialize(sDxCommon);

	sObject3dCommon = new Object3dCommon();
	sObject3dCommon->Initialize(sDxCommon);

	// カメラ初期化
	CameraManager::GetInstance()->Initialize();
	CameraManager::GetInstance()->CreateCamera("Global");
	CameraManager::GetInstance()->SetActiveCamera("Global");

	// --- 初期化コマンド転送完了待機 ---
	hr = sDxCommon->GetCommandList()->Close();
	assert(SUCCEEDED(hr));
	ID3D12CommandList* commandLists[] = { sDxCommon->GetCommandList() };
	sDxCommon->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	// Fence処理
	sDxCommon->IncrementFenceValue();
	sDxCommon->GetCommandQueue()->Signal(sDxCommon->GetFence(), sDxCommon->GetFenceValue());
	if (sDxCommon->GetFence()->GetCompletedValue() < sDxCommon->GetFenceValue()) {
		sDxCommon->GetFence()->SetEventOnCompletion(sDxCommon->GetFenceValue(), sDxCommon->GetFenceEvent());
		WaitForSingleObject(sDxCommon->GetFenceEvent(), INFINITE);
	}

	// 次のフレーム用にリセット
	hr = sDxCommon->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = sDxCommon->GetCommandList()->Reset(sDxCommon->GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(hr));
}

bool TDEngine::Update() {
	// ウィンドウメッセージ処理
	if (sWinApp->ProcessMessage()) {
		return true;
	}

	// 入力更新
	sInput->Update();

	// カメラ更新
	CameraManager::GetInstance()->Update();


	return false;
}

void TDEngine::Finalize() {
	// 各種解放処理
	if (sImGuiManager) sImGuiManager->Finalize();

	if (sAudioManager) {
		// 必要ならUnloadAll的な処理
		delete sAudioManager;
	}

	// シングルトンの終了処理
	ParticleManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	CameraManager::GetInstance()->Finalize();

	if (sObject3dCommon) delete sObject3dCommon;
	if (sSpriteCommon) delete sSpriteCommon;
	if (sSrvManager) delete sSrvManager;

	CloseHandle(sDxCommon->GetFenceEvent());



	if (sInput) delete sInput;
	if (sWinApp) delete sWinApp;
}