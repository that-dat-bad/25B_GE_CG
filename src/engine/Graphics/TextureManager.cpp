#include "TextureManager.h"
#include "DirectXCommon.h"
#include <cassert>
#include <filesystem> 
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
	std::wstring wFilePath = ConvertString(filePath);


	hr = DirectX::LoadFromWICFile(wFilePath.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

	assert(SUCCEEDED(hr));

	DirectX::ScratchImage mipImages;
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr));
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

