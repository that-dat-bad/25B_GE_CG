#pragma once
#include "DirectXCommon.h"
namespace TDEngine {
class ModelCommon
{
public:
	//初期化
	void Initialize(DirectXCommon* dxCommon);

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }
private:
	DirectXCommon* dxCommon_;
};
}

