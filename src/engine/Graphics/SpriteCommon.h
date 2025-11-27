#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <array>
class DirectXCommon;

enum class BlendMode {
	kNone,
	kNormal,
	kAdd,
	kSubtract,
	kMultiply,
	kScreen,

	kCountBlendMode
};

class SpriteCommon
{
public:
	void Initialize(DirectXCommon* dxCommon);

	//共通描画設定
	void SetupCommonState();

	void SetBlendMode(BlendMode blendMode);

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }

private:
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCountBlendMode)> graphicsPipelineStates_;
	//ルートシグネチャの作成
	void CreateRootSignature(DirectXCommon* dxCommon);

	//グラフィックパイプラインの生成
	void CreateGraphicsPipeline(DirectXCommon* dxCommon);
};

