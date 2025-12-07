#pragma once
#include <cstdint>
#include <wrl/client.h>
#include <d3d12.h>
namespace TDEngine {
	class DirectXCommon;

	class SrvManager {
	public:
		static SrvManager* GetInstance();
		void Initialize(DirectXCommon* dxCommon);

		void PreDraw();

		void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

		uint32_t Allocate();

		//テクスチャ枚数上限チェック
		bool CanAllocate() const;

		//最大SRV数(最大テクスチャ枚数)
		static const uint32_t kMaxSRVCount_;
		//SRV用のデスクリプタサイズ
		uint32_t descriptorSize_;
		//SRV用デスクリプタヒープ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

		//SRV生成(テクスチャ用)
		void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
		void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
	private:

		SrvManager() = default;
		~SrvManager() = default;
		SrvManager(const SrvManager&) = delete;
		SrvManager& operator=(const SrvManager&) = delete;

		DirectXCommon* dxCommon_ = nullptr;
		//次に使用するSRVインデックス
		uint32_t useIndex_ = 0;
	};

}