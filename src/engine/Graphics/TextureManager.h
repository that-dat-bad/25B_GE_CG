#pragma once
#include <string>
#include "../../../external/DirectXTex/DirectXTex.h"
#include <wrl/client.h>
#include <d3d12.h>
#include <vector>

// 前方宣言
class DirectXCommon;

class TextureManager
{
public:
	static TextureManager* GetInstance();

	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	// 終了処理
	void Finalize();

	// テクスチャ読み込み
	void LoadTexture(const std::string& filePath);

	// SRVハンドル(GPU)を取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

private:
	static TextureManager* instance_;
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	struct TextureData
	{
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};
	std::vector<TextureData> textureDatas_;

	DirectXCommon* dxCommon_ = nullptr;

	static uint32_t kSRVIndexTop;
};