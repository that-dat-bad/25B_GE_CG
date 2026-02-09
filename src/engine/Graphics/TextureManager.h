#pragma once
#include <string>
#include "../../../external/DirectXTex/DirectXTex.h"
#include <wrl/client.h>
#include <d3d12.h>
#include<unordered_map>


class SrvManager;
class DirectXCommon;

#include <memory> 

class TextureManager
{
public:
	static TextureManager* GetInstance();

	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	void Finalize();

	void LoadTexture(const std::string& filePath);
	void Load(const std::string& filePath) { LoadTexture(filePath); }

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);

	~TextureManager() = default;

private:



	static std::unique_ptr<TextureManager> instance_;
	TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	struct TextureData
	{
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};
	std::unordered_map<std::string,TextureData> textureDatas_;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

};
