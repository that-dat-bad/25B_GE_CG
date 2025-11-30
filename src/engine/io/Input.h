#pragma once
#include <Windows.h>
#include <wrl/client.h>
#include <dinput.h>
#include<stdint.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class Input
{
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

	// --- キーボード ---
	bool pushKey(BYTE keyNumber);
	bool triggerKey(BYTE keyNumber);

	// --- マウス ---
	// ボタン押しっぱなし判定 (0:左, 1:右, 2:中, 3~7:その他)
	bool PushMouse(uint32_t buttonNumber);
	// ボタンのトリガー判定 (押した瞬間)
	bool TriggerMouse(uint32_t buttonNumber);

	// マウス移動量取得 (前回フレームからの移動量)
	long GetMouseMoveX() const;
	long GetMouseMoveY() const;
	long GetMouseWheel() const; // ホイール回転量

private:
	ComPtr<IDirectInput8> directInput_;
	ComPtr<IDirectInputDevice8> keyboard_;
	ComPtr<IDirectInputDevice8> mouse_;

	// キーボード情報
	BYTE keys_[256] = {};
	BYTE preKeys_[256] = {};

	// マウス情報
	DIMOUSESTATE2 mouseState_ = {};    // 現在のマウス状態
	DIMOUSESTATE2 preMouseState_ = {}; // 1フレーム前のマウス状態
};