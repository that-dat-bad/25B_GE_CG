#pragma once
#include <Windows.h>
#include <string>
#include <cstdint>


#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "ImGuiManager.h"

namespace TDEngine {

	// グローバル初期化
	// title: ウィンドウタイトル
	// width, height: 画面解像度 (デフォルト引数付き)
	void Initialize(const std::wstring& title, int width = 1280, int height = 720);

	// エンジン更新処理 (ウィンドウメッセージ処理など)
	// 戻り値: trueならアプリ終了
	bool Update();

	// 終了処理
	void Finalize();
}