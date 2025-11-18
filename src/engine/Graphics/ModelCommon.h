#pragma once
class DirectXCommon;
class ModelCommon
{
public:
	void Initialize(DirectXCommon* dxCommon);


	void GetDirectXCommon(DirectXCommon* dxCommon) { dxCommon_ = dxCommon; }

private:
	DirectXCommon* dxCommon_;
};

