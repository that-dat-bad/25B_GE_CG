#include <Windows.h>
#include <cassert>
#include <mfapi.h>
#pragma comment(lib, "mfplat.lib")
#include "../engine/Debug/dump.h"
#include "Game.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	Dump::Install();

	HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(hr));

	Game game;
	game.Initialize();

	game.Run();

	game.Finalize();

	hr = MFShutdown();
	assert(SUCCEEDED(hr));

	return 0;
}
