#include "Sprite.h"
#include"SpriteCommon.h"
#include"DirectXCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, DirectXCommon* dxCommon)
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
	VertexData* vertexDataSprite = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	float initialSpriteWidth = 100.0f;
	float initialSpriteHeight = 100.0f;

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
	materialData_->enableLighting = false;
	materialData_->uvTransform = Identity4x4();


	//座標変換行列リソースの作成
	wvpResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//座標変換行列リソースにデータを書き込むためのアドレスを取得してtransformationMatrixDataに割り当てる
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	//単位行列を書き込んでおく
	transformationMatrixData_->WVP = Identity4x4();
	transformationMatrixData_->World = Identity4x4();
}

void Sprite::Update()
{
	vertexDataSprite[0].position = { 0.0f, currentSpriteHeight, 0.0f, 1.0f };    vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };       vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[2].position = { currentSpriteWidth, currentSpriteHeight, 0.0f, 1.0f };  vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[3].position = { currentSpriteWidth, 0.0f, 0.0f, 1.0f };     vertexDataSprite[3].texcoord = { 1.0f, 0.0f };



	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
}