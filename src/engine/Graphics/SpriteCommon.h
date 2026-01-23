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

	//共通描画設定
	void SetupCommonState();

	// ブレンドモード設定
	void SetBlendMode(BlendMode mode);

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }

private:
	static std::unique_ptr<SpriteCommon> instance;
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCountOf)> graphicsPipelineStates_;
	
	//ルートシグネチャの作成
	void CreateRootSignature(DirectXCommon* dxCommon);

	//グラフィックパイプラインの生成
	void CreateGraphicsPipeline(DirectXCommon* dxCommon);
};

