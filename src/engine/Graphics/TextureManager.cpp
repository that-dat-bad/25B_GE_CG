#include "TextureManager.h"
#include "DirectXCommon.h"
#include <cassert>
#include <filesystem>
#include <vector>
#include"SrvManager.h"
#include <memory> // Required for std::unique_ptr

std::unique_ptr<TextureManager> TextureManager::instance_ = nullptr;


std::wstring ConvertString(const std::string& str) {
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

TextureManager* TextureManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_.reset(new TextureManager());
	}
	return instance_.get();
}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	textureDatas_.reserve(SrvManager::kMaxSRVCount_);
}

void TextureManager::Finalize()
{
	instance_.reset();
}

void TextureManager::LoadTexture(const std::string& filePath) {
	HRESULT hr;


	if (textureDatas_.contains(filePath)) {
		return;
	}

	assert(srvManager_->CanAllocate());

	DirectX::ScratchImage image;
	
	// 探索候補のリスト作成
	std::vector<std::string> searchPaths;
	searchPaths.push_back(filePath);

	std::filesystem::path p(filePath);
	// 相対パスの場合のみ、ベースディレクトリを付与したパスを候補に追加
	if (p.is_relative()) {
		searchPaths.push_back("Resources/" + filePath);
		searchPaths.push_back("assets/textures/" + filePath);
	}

	// ファイル名のみでも試す（パスが含まれている場合）
	std::string filename = p.filename().string();
	if (filename != filePath) {
		searchPaths.push_back("Resources/" + filename);
		searchPaths.push_back("assets/textures/" + filename);
	}

	hr = E_FAIL;
	bool found = false;
	for (const std::string& path : searchPaths) {
		if (std::filesystem::exists(path)) {
			std::wstring wPath = ConvertString(path);
			hr = DirectX::LoadFromWICFile(wPath.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
			if (SUCCEEDED(hr)) {
				found = true;
				break;
			}
		}
	}

	// 全ての候補で見つからなかった場合
	if (!found) {
		std::string errorMsg = "Texture Load Failed: " + filePath + "\nSearched in:\n";
		for (const auto& path : searchPaths) {
			errorMsg += "  - " + path + "\n";
		}
		// Windowsのデバッグ出力に表示
		OutputDebugStringA(errorMsg.c_str());
		// アサートで停止
		assert(found && "Texture not found in any search path.");
	}

	DirectX::ScratchImage mipImages;
	// 圧縮フォーマット、または既に 1x1 の場合はミップマップ生成をスキップ
	if (DirectX::IsCompressed(image.GetMetadata().format) || 
		(image.GetMetadata().width == 1 && image.GetMetadata().height == 1)) {
		mipImages = std::move(image);
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		// 万が一ミップマップ生成に失敗した場合は、元の画像を使用して続行する
		if (FAILED(hr)) {
			mipImages = std::move(image);
		}
	}


	TextureData& textureData = textureDatas_[filePath];


	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(dxCommon_->GetDevice(), textureData.metadata);

	textureData.intermediateResource = dxCommon_->UploadTextureData(textureData.resource.Get(), mipImages);


	textureData.srvIndex = srvManager_->Allocate();

	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	srvManager_->CreateSRVforTexture2D(
		textureData.srvIndex,
		textureData.resource.Get(),
		textureData.metadata.format,
		UINT(textureData.metadata.mipLevels)
	);
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) {
	return srvManager_->GetGPUDescriptorHandle(textureIndex);
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	if (textureDatas_.contains(filePath)) {
		return textureDatas_[filePath].metadata;
	}

	assert(false && "Texture not found.");
	static DirectX::TexMetadata dummy{};
	return dummy;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath) {
	if (textureDatas_.contains(filePath)) {
		return textureDatas_[filePath].srvIndex;
	}

	assert(false && "Texture not found.");
	return 0;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex) {
	for (const auto& [key, data] : textureDatas_) {
		if (data.srvIndex == textureIndex) {
			return data.metadata;
		}
	}

	assert(false && "Texture not found.");
	static DirectX::TexMetadata dummy{};
	return dummy;
}

