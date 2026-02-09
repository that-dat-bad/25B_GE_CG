#pragma once
#include <cstdint>
#include <wrl/client.h>
#include <d3d12.h>
class DirectXCommon;

class SrvManager {
public:
	void Initialize(DirectXCommon* dxCommon);

	void PreDraw();

	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	uint32_t Allocate();

	bool CanAllocate() const;

	static const uint32_t kMaxSRVCount_;
	uint32_t descriptorSize_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
private:
	DirectXCommon* dxCommon_ = nullptr;
	uint32_t useIndex_ = 0;
};


