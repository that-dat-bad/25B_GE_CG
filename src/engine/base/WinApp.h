#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp {
public:
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public:
	void Initialize();

	bool ProcessMessage();

	void Finalize();

	HWND GetHwnd() const { return hwnd_; }

private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	HWND hwnd_ = nullptr;
	WNDCLASS wc_{};
};
