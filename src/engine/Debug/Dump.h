#pragma once
#include <Windows.h>

class Dump {
public:
	static void Install();
	static void Uninstall();

private:
	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);
};
