#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "../base/Math/MyMath.h"
#include "TextureManager.h"
#include "../../../external/DirectXTex/DirectXTex.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	spriteCommon_ = spriteCommon;
	DirectXCommon* dxCommon = spriteCommon_->GetDirectXCommon();
	assert(dxCommon != nullptr && "dxCommon is NULL! SpriteCommon::Initialize was not called.");
	assert(dxCommon->GetDevice() != nullptr && "D3D12Device is NULL!");
	vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
	indexBuffer_ = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);

	vertexBufferView.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	indexBufferView.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	uint32_t* indexDataLocal = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataLocal));
	indexDataLocal[0] = 0; indexDataLocal[1] = 1; indexDataLocal[2] = 2;
	indexDataLocal[3] = 1; indexDataLocal[4] = 3; indexDataLocal[5] = 2;
	indexBuffer_->Unmap(0, nullptr);

	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->WVP = Identity4x4();

	if (textureFilePath.empty()) {
		textureIndex_ = 0;
		size_ = { 100.0f, 100.0f }; 
	}
	else {
		textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
		AdjustTextureSize();
	}

	anchorPoint_ = { 0.5f, 0.5f };
}

void Sprite::Update()
{
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	const DirectX::TexMetadata& metaData = TextureManager::GetInstance()->GetMetaData(textureIndex_);

	float texWidth = static_cast<float>(metaData.width);
	float texHeight = static_cast<float>(metaData.height);
	if (texWidth == 0) texWidth = 1.0f;
	if (texHeight == 0) texHeight = 1.0f;

	float texLeft = textureLeftTop_.x / texWidth;
	float texRight = (textureLeftTop_.x + textureSize_.x) / texWidth;
	float texTop = textureLeftTop_.y / texHeight;
	float texBottom = (textureLeftTop_.y + textureSize_.y) / texHeight;

	if (isFlipX_) { left = -left; right = -right; }
	if (isFlipY_) { top = -top; bottom = -bottom; }

	vertexData_[0].position = { left, bottom, 0.0f, 1.0f }; 
	vertexData_[0].texcoord = { texLeft,texBottom };
	vertexData_[1].position = { left, top, 0.0f, 1.0f };   
	vertexData_[1].texcoord = { texLeft, texTop };
	vertexData_[2].position = { right, bottom, 0.0f, 1.0f };
	vertexData_[2].texcoord = { texRight, texBottom };
	vertexData_[3].position = { right, top, 0.0f, 1.0f };  
	vertexData_[3].texcoord = { texRight, texTop };

	Vector3 finalScale = {
		size_.x * transform_.scale.x,
		size_.y * transform_.scale.y,
		1.0f
	};

	Matrix4x4 worldMatrix = MakeAffineMatrix(finalScale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = Identity4x4();
	Matrix4x4 projectionMatrix = makeOrthographicmMatrix(0.0f, 0.0f, 1280.0f, 720.0f, -10.0f, 100.0f);
	Matrix4x4 wvpMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	materialData_->WVP = wvpMatrix;
}

void Sprite::Draw()
{
	DirectXCommon* dxCommon = spriteCommon_->GetDirectXCommon();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	
	spriteCommon_->SetBlendMode(blendMode_);

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));

	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::ChangeTexture(std::string textureFilePath)
{
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	AdjustTextureSize();
}

void Sprite::AdjustTextureSize()
{
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureIndex_);
	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);
	size_ = textureSize_;
}
