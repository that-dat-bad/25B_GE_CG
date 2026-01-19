#include "Dump.h"
#include <Windows.h>
#include <strsafe.h>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

void Dump::Install() {
	SetUnhandledExceptionFilter(ExportDump);
}

void Dump::Uninstall() {
	SetUnhandledExceptionFilter(nullptr);
}

LONG WINAPI Dump::ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dump", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dump/%04d-%02d-%02d-%02d-%02d.dmp",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);

	HANDLE dumpFileHandle = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (dumpFileHandle == INVALID_HANDLE_VALUE) {
		return EXCEPTION_EXECUTE_HANDLER;
	}

	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{};
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

	CloseHandle(dumpFileHandle);
	return EXCEPTION_EXECUTE_HANDLER;
}
