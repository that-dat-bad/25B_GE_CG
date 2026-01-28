#pragma once
class DirectXCommon;
class ModelCommon
{
public:
	void Initialize(DirectXCommon* dxCommon);


	DirectXCommon* GetDirectXCommon() { return dxCommon_; }
private:
	DirectXCommon* dxCommon_;
};


