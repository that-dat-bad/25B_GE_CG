#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <array>
#include "BlendMode.h"
#include <memory>

class DirectXCommon;
class SrvManager;

class SkyboxCommon
{
public:
	static SkyboxCommon* GetInstance();

	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	void Finalize();

	// 共通描画設定
	void SetupCommonState();

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }
	SrvManager* GetSrvManager() { return srvManager_; }

	~SkyboxCommon() = default;
private:
	SkyboxCommon() = default;
	SkyboxCommon(const SkyboxCommon&) = delete;
	SkyboxCommon& operator=(const SkyboxCommon&) = delete;

	static std::unique_ptr<SkyboxCommon> instance_;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// ルートシグネチャの作成
	void CreateRootSignature(DirectXCommon* dxCommon);

	// グラフィックパイプラインの生成
	void CreateGraphicsPipeline(DirectXCommon* dxCommon);
};
