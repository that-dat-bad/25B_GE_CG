#include <Windows.h>
#include <cassert>
#include <strsafe.h>
#include <DbgHelp.h>
#include <mfapi.h>
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "mfplat.lib")
#include "Game.h"

// ミニダンプ出力用
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dump", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dump/%04d-%02d-%02d-%02d-%02d.dmp",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
	return EXCEPTION_EXECUTE_HANDLER;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	// 例外ダンプハンドラ登録
	SetUnhandledExceptionFilter(ExportDump);

	// Media Foundation 初期化
	HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(hr));

	// Game 実行
	Game game;
	game.Initialize();

	while (!game.IsEndRequest()) {
		game.Update();
		game.Draw();
	}

	game.Finalize();

	hr = MFShutdown();
	assert(SUCCEEDED(hr));

	return 0;
}