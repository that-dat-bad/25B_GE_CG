#include "TextureManager.h"
#include "DirectXCommon.h"
#include <cassert>
#include <filesystem> 
#include"SrvManager.h"
TextureManager* TextureManager::instance_ = nullptr;
uint32_t TextureManager::kSRVIndexTop = 1;

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
	delete instance_;
	instance_ = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath) {
	HRESULT hr;
	//auto it = std::find_if(
	//	textureDatas_.begin(),
	//	textureDatas_.end(),
	//	[&](TextureData& textureData) { return textureData.filePath == filePath; }
	//);

	//if (it != textureDatas_.end()) {
	//	return; // 読み込み済みなら早期return
	//}

	if (textureDatas_.contains(filePath)) {
		return;
	}
	//assert(textureDatas_.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount_);

	assert(srvManager_->)

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


	// --- テクスチャデータを追加 ---
	textureDatas_.resize(textureDatas_.size() + 1);
	// 追加した要素の参照を取得する
	TextureData& textureData = textureDatas_[filePath];


	// ---  テクスチャデータ書き込み ---
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(dxCommon_->GetDevice(), textureData.metadata);

	// ---テクスチャデータ転送 ---
	textureData.intermediateResource = dxCommon_->UploadTextureData(textureData.resource.Get(), mipImages);


	// ---  デスクリプタハンドルの計算 ---
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas_.size() - 1) + kSRVIndexTop;

	ID3D12DescriptorHeap* srvHeap = dxCommon_->GetSRVDescriptorHeap();
	uint32_t descriptorSize = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);


	// ---SRVの生成 ---
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) {
	//範囲外指定違反チェック
	assert(textureIndex < textureDatas_.size());
	TextureData& textureData = textureDatas_[textureIndex];
	return textureDatas_[textureIndex].srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	// 線形探索で探す
	auto it = std::find_if(
		textureDatas_.begin(),
		textureDatas_.end(),
		[&](TextureData& textureData) { return textureData.filePath == filePath; }
	);

	if (it != textureDatas_.end()) {
		return it->metadata;
	}

	assert(false && "Texture not found.");
	static DirectX::TexMetadata dummy{};
	return dummy;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath) {
	auto it = std::find_if(
		textureDatas_.begin(),
		textureDatas_.end(),
		[&](TextureData& textureData) { return textureData.filePath == filePath; }
	);
	if (it != textureDatas_.end()) {
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas_.begin(), it));
		return textureIndex;
	}
	assert(0);
	return 0;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex) {
	//範囲外指定違反チェック
	assert(textureIndex < textureDatas_.size());
	TextureData& textureData = textureDatas_[textureIndex];
	return textureData.metadata;
}
