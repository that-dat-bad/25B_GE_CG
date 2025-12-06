#pragma once
#include <TDEngine.h>

struct AABB {
	TDEngine::Vector3 min; // 最小点
	TDEngine::Vector3 max; // 最大点
};