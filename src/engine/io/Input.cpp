#define DIRECTINPUT_VERSION 0x0800
#include "Input.h"
#include <dinput.h>
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	HRESULT result;

	// インスタンスの生成
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データのセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// マウスデバイスの生成
	result = directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(result));

	// 入力データのセット
	result = mouse_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	mouse_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));
}

void Input::Update()
{
	// --- キーボード更新 ---
	// 前回の入力を保存
	memcpy(preKeys_, keys_, sizeof(keys_));

	// デバイス取得権限の確認
	keyboard_->Acquire();
	// 状態取得
	keyboard_->GetDeviceState(sizeof(keys_), keys_);


	// --- マウス更新 ---
	// 前回の入力を保存
	preMouseState_ = mouseState_;

	// デバイス取得権限の確認
	mouse_->Acquire();
	// 状態取得 (メンバ変数に保存)
	mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &mouseState_);
}

bool Input::pushKey(BYTE keyNumber)
{
	if (keys_[keyNumber])
	{
		return true;
	}
	return false;
}

bool Input::triggerKey(BYTE keyNumber)
{
	if (!preKeys_[keyNumber] && keys_[keyNumber])
	{
		return true;
	}
	return false;
}

// --- マウスの実装 ---

bool Input::PushMouse(uint32_t buttonNumber)
{
	if ((mouseState_.rgbButtons[buttonNumber] & 0x80)) {
		return true;
	}
	return false;
}

bool Input::TriggerMouse(uint32_t buttonNumber)
{
	// 前回押されてなくて、今回押されていればトリガー
	if (!(preMouseState_.rgbButtons[buttonNumber] & 0x80) &&
		(mouseState_.rgbButtons[buttonNumber] & 0x80)) {
		return true;
	}
	return false;
}

long Input::GetMouseMoveX() const
{
	return mouseState_.lX;
}

long Input::GetMouseMoveY() const
{
	return mouseState_.lY;
}

long Input::GetMouseWheel() const
{
	return mouseState_.lZ;
}