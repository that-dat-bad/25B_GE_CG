#pragma once
#include <TDEngine.h>

struct OBB
{
	TDEngine::Vector3 center;    // 中心点
	TDEngine::Vector3 orientations[3]; // 各軸の方向ベクトル
	TDEngine::Vector3 size;
};