#pragma once
#include <d3d12.h>
#include <wrl/client.h>
class DirectXCommon;

class SpriteCommon
{
public:
	static SpriteCommon* GetInstance();
    void Initialize(DirectXCommon* dxCommon);

	//共通描画設定
	void SetupCommonState();

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }

private:
	static SpriteCommon* instance;
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
	//ルートシグネチャの作成
	void CreateRootSignature(DirectXCommon* dxCommon);

	//グラフィックパイプラインの生成
	void CreateGraphicsPipeline(DirectXCommon* dxCommon);
};

