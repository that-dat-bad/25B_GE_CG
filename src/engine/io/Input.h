#pragma once
#ifndef ENGINE_IO_INPUT_H
#define ENGINE_IO_INPUT_H

#include<Windows.h>
#include <wrl/client.h>
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <memory>

#ifdef Input
#undef Input
#endif

class Input
{
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	static Input* GetInstance();

	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
	struct MouseMove {
		long lX;
		long lY;
		long lZ;
	};
	MouseMove GetMouseMove();
	// bool IsPressMouse(int buttonNumber);
	// bool IsTriggerMouse(int buttonNumber);
	bool PushMouse(int buttonNumber);
	~Input() = default;

private:
	Input() = default;
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
	static std::unique_ptr<Input> instance_;

	ComPtr<IDirectInput8> directInput_;
	ComPtr<IDirectInputDevice8> keyboard_;
	ComPtr<IDirectInputDevice8> mouse_;
	BYTE keys_[256] = {};
	BYTE preKeys_[256] = {};
	DIMOUSESTATE2 mouseState_ = {};
	// DIMOUSESTATE2 preMouseState_ = {};
};


#endif // ENGINE_IO_INPUT_H

