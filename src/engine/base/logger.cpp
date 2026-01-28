#include <Windows.h>
#include "logger.h"
#include <debugapi.h>

namespace logger {
	void Log(const std::string& message) {
		OutputDebugStringA(message.c_str());
	}
}
