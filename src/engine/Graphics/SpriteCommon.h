#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <array>
#include "BlendMode.h"
class DirectXCommon;

#include <memory> 

class SpriteCommon
{
public:
	static SpriteCommon* GetInstance();
    void Initialize(DirectXCommon* dxCommon);

	void SetupCommonState();

	void SetBlendMode(BlendMode mode);

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }

private:
	static std::unique_ptr<SpriteCommon> instance;
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCountOf)> graphicsPipelineStates_;
	
	void CreateRootSignature(DirectXCommon* dxCommon);

	void CreateGraphicsPipeline(DirectXCommon* dxCommon);
};


