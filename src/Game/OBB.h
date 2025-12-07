#pragma once
#include <TDEngine.h>

struct OBB
{
	MyMath::Vector3 center;    // 中心点
	MyMath::Vector3 orientations[3]; // 各軸の方向ベクトル
	MyMath::Vector3 size;
};