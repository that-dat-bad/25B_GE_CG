#include "Sprite.h"
#include"SpriteCommon.h"
#include"DirectXCommon.h"
#include"../base/Math/MyMath.h"
#include"TextureManager.h"
#include"../../../external/DirectXTex/DirectXTex.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, DirectXCommon* dxCommon, std::string textureFilePath)
{
	spriteCommon_ = spriteCommon;

	//vertexResource(vertexBuffer)を作る
	vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
	//IndexResource(indexBuffer)を作る
	indexBuffer_ = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	//VertexBufferViewの作成
	vertexBufferView.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//IndexBufferViewの作成
	indexBufferView.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	float initialSpriteWidth = 100.0f;
	float initialSpriteHeight = 100.0f;
	size_.x = initialSpriteWidth;
	size_.y = initialSpriteHeight;

	//IndexResourceにデータを書き込むためのアドレスを取得してindexDataに割り当てる
	uint32_t* indexDataLocal = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataLocal));
	indexDataLocal[0] = 0; indexDataLocal[1] = 1; indexDataLocal[2] = 2;
	indexDataLocal[3] = 1; indexDataLocal[4] = 3; indexDataLocal[5] = 2;
	indexBuffer_->Unmap(0, nullptr);


	//マテリアルリソースの作成
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->WVP = Identity4x4();
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	anchorPoint_ = { 0.5f, 0.5f };
	AdjustTextureSize();
}

void Sprite::Update()
{


	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	const DirectX::TexMetadata& metaData = TextureManager::GetInstance()->GetMetaData(textureIndex_);

	float texLeft = textureLeftTop_.x / static_cast<float>(metaData.width);
	float texRight = (textureLeftTop_.x + textureSize_.x) / static_cast<float>(metaData.width);
	float texTop = textureLeftTop_.y / static_cast<float>(metaData.height);
	float texBottom = (textureLeftTop_.y + textureSize_.y) / static_cast<float>(metaData.height);

	//左右反転
	if (isFlipX_)
	{
		left = -left;
		right = -right;
	}

	//上下反転
	if (isFlipY_)
	{
		top = -top;
		bottom = -bottom;
	}

	vertexData_[0].position = { left, bottom, 0.0f, 1.0f }; // 左下
	vertexData_[0].texcoord = { texLeft,texBottom };
	vertexData_[1].position = { left, top, 0.0f, 1.0f };   // 左上
	vertexData_[1].texcoord = { texLeft, texTop };
	vertexData_[2].position = { right, bottom, 0.0f, 1.0f };// 右下
	vertexData_[2].texcoord = { texRight, texBottom };
	vertexData_[3].position = { right, top, 0.0f, 1.0f };  // 右上
	vertexData_[3].texcoord = { texRight, texTop };

	Vector3 finalScale = {
		size_.x * transform_.scale.x,
		size_.y * transform_.scale.y,
		1.0f
	};
	// 2. 行列計算
	//Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 worldMatrix = MakeAffineMatrix(finalScale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = Identity4x4();
	Matrix4x4 projectionMatrix = makeOrthographicmMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);

	Matrix4x4 wvpMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	// 3. マテリアルへの書き込み
	materialData_->WVP = wvpMatrix;

}

void Sprite::Draw(DirectXCommon* dxCommon)
{
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	// 1. 頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 2. インデックスバッファの設定
	commandList->IASetIndexBuffer(&indexBufferView);

	// 3. マテリアル(定数バッファ)の設定
	// [0]: マテリアル(WVP+Color)  [1]: テクスチャ
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// 4. テクスチャの設定
	//commandList->SetGraphicsRootDescriptorTable(1, textureSrvHandle);
	commandList->SetGraphicsRootDescriptorTable(1,TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));
	

	// 5. 描画コマンド
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::ChangeTexture(std::string textureFilePath)
{
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
}

void Sprite::AdjustTextureSize()
{
	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureIndex_);
	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);
	//画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}
