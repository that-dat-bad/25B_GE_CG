#pragma once

#include <string>
#include <wrl.h>
#include "../../../external/DirectXTex/DirectXTex.h"
#include "../../../external/DirectXTex/d3dx12.h"

struct ID3D12Resource;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;

// 読み込んだテクスチャアセット
struct TextureAsset {
	std::string name;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	DirectX::TexMetadata metadata;
};