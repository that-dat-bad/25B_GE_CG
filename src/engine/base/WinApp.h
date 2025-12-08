#pragma once
#include <Windows.h>
#include <cstdint>
#include<string>

class WinApp {
public:
	// クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public:
	// 初期化
	void Initialize(const std::wstring& title = L"TDengine");

	bool ProcessMessage();

	// 終了
	void Finalize();

	// ウィンドウハンドルの取得
	HWND GetHwnd() const { return hwnd_; }

	HINSTANCE GetInstance() const;

private:
	// ウィンドウプロシージャ
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	HWND hwnd_ = nullptr;
	WNDCLASS wc_{};
};