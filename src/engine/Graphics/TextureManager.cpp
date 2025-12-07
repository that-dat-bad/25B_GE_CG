#include "TextureManager.h"
#include "DirectXCommon.h"
#include <cassert>
#include <filesystem> 
#include "SrvManager.h"
using namespace TDEngine;
TextureManager* TextureManager::instance_ = nullptr;


std::wstring ConvertString(const std::string& str) {
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

TextureManager* TextureManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new TextureManager();
	}
	return instance_;
}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	textureDatas_.reserve(128);
}

void TextureManager::Finalize() {
	textureDatas_.clear();
	delete instance_;
	instance_ = nullptr;
}

uint32_t TextureManager::Load(const std::string& filePath) {
	return TextureManager::GetInstance()->LoadInternal(filePath);
}


uint32_t TextureManager::LoadTexture(const std::string& filePath) {
	return TextureManager::GetInstance()->LoadInternal(filePath);
}


uint32_t TextureManager::LoadInternal(const std::string& filePath) {
	HRESULT hr;

	// 読み込み済みならインデックスを返す
	if (textureDatas_.contains(filePath)) {
		return textureDatas_[filePath].srvIndex;
	}

	assert(srvManager_->CanAllocate());

	DirectX::ScratchImage image;
	std::wstring wFilePath = ConvertString(filePath);

	hr = DirectX::LoadFromWICFile(wFilePath.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	DirectX::ScratchImage mipImages;
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	}
	else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr));
	}


	// --- テクスチャデータを追加 ---
	TextureData& textureData = textureDatas_[filePath];


	// ---  テクスチャデータ書き込み ---
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(dxCommon_->GetDevice(), textureData.metadata);

	// ---テクスチャデータ転送 ---
	textureData.intermediateResource = dxCommon_->UploadTextureData(textureData.resource.Get(), mipImages);


	// ---  デスクリプタハンドルの計算 ---
	textureData.srvIndex = srvManager_->Allocate();

	//ハンドルを取得して保存
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	//SRV生成
	srvManager_->CreateSRVforTexture2D(
		textureData.srvIndex,
		textureData.resource.Get(),
		textureData.metadata.format,
		UINT(textureData.metadata.mipLevels)
	);

	// インデックスを返す
	return textureData.srvIndex;
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