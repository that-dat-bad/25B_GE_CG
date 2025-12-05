#pragma once
#include <string>
#include <d3d12.h>

// 各種マネージャのヘッダー
#include "WinApp.h"
#include "../Graphics/DirectXCommon.h"
#include "../io/Input.h"
#include "../Audio/AudioManager.h"
#include "../Graphics/TextureManager.h"
#include "../Graphics/SrvManager.h"
#include "../Graphics/ModelManager.h"
#include "../Graphics/SpriteCommon.h"
#include "../Graphics/Object3dCommon.h"
#include "../Debug/ImGuiManager.h"
#include "../Graphics/ParticleManager.h"

// TDEngine 名前空間
namespace TDEngine
{
	// エンジンの初期化
	void Initialize(const std::wstring& title, int width, int height);

	// エンジンの更新（フレーム開始処理・入力更新など）
	// 戻り値: falseならゲーム終了 (ウィンドウが閉じられた等)
	bool Update();

	// 描画開始（画面クリア・描画前処理）
	void BeginFrame();

	// 描画終了（ImGui描画・画面フリップ・FPS固定）
	void EndFrame();

	// エンジンの終了処理（リソース解放）
	void Finalize();

	// --- ゲッター（ユーザーが細かい機能を使いたい場合用） ---
	WinApp* GetWinApp();
	DirectXCommon* GetDXCommon();
	SpriteCommon* GetSpriteCommon();
	Object3dCommon* GetObject3dCommon();
	Input* GetInput();
	AudioManager* GetAudio();
}