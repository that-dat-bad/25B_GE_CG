#pragma once
#include<Windows.h>
#include <wrl/client.h>
#define DIRECTINPUT_VERSION     0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <memory>

class Input
{
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	static Input* GetInstance();

	void Initialize(HINSTANCE hInstance, HWND hwnd);

	/// マウスカーソルをウィンドウ中央にロック＆非表示にする
	void LockCursor();
	/// マウスカーソルのロックを解除＆表示する
	void UnlockCursor();
	/// カーソルがロックされているか
	bool IsCursorLocked() const { return cursorLocked_; }
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
	struct MouseMove {
		long lX;
		long lY;
		long lZ;
	};
	MouseMove GetMouseMove();
	bool PushMouse(int buttonNumber);

	// スクリーン座標でのマウス位置取得（クライアント領域基準）
	struct MousePosition {
		long x;
		long y;
	};
	MousePosition GetMouseScreenPosition(HWND hwnd);
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

	HWND hwnd_ = nullptr;           // ウィンドウハンドル（カーソル制御用）
	bool cursorLocked_ = false;     // カーソルロック状態
};

