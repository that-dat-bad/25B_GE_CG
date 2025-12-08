#define DIRECTINPUT_VERSION 0x0800
#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	HRESULT result;

	// DirectInputオブジェクトの生成
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// マウスデバイスの生成
	result = directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = mouse_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	mouse_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));
}

void Input::Update() {
	memcpy(preKeys_, keys_, sizeof(keys_));

	// 2. キーボード情報の取得
	HRESULT hr = keyboard_->Acquire(); // デバイス取得
	if (SUCCEEDED(hr) || hr == DI_NOEFFECT) {
		// 状態取得
		keyboard_->GetDeviceState(sizeof(keys_), keys_);
	} else {
		memset(keys_, 0, sizeof(keys_));
		// 再度取得を試みる（次回用）
		keyboard_->Acquire();
	}

	// 3. マウス情報の取得
	mouse_->Acquire();
	mouse_->GetDeviceState(sizeof(mouseState_), &mouseState_);
}

bool Input::pushKey(BYTE keyNumber) {
	// 0x80 (最上位ビット) が立っていれば押されている
	if (keys_[keyNumber] & 0x80) {
		return true;
	}
	return false;
}

bool Input::triggerKey(BYTE keyNumber) {
	bool isPressed = (keys_[keyNumber] & 0x80) != 0;
	bool wasPressed = (preKeys_[keyNumber] & 0x80) != 0;

	if (isPressed && !wasPressed) {
		return true;
	}

	return false;
}

bool Input::PushMouse(int buttonNumber) const {
	if (mouseState_.rgbButtons[buttonNumber] & 0x80) {
		return true;
	}
	return false;
}

long Input::GetMouseMoveX() const {
	return mouseState_.lX;
}

long Input::GetMouseMoveY() const {
	return mouseState_.lY;
}

long Input::GetWheel() const {
	return mouseState_.lZ;
}