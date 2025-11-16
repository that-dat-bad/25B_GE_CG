#include "Sprite.h"
#include"SpriteCommon.h"
#include"DirectXCommon.h"
#include"../base/Math/MyMath.h"
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
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
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
    materialData_->WVP = Identity4x4();

}

void Sprite::Update()
{

    float width = 100.0f;
    float height = 100.0f;

    vertexData_[0].position = { 0.0f, height, 0.0f, 1.0f }; // 左下
    vertexData_[0].texcoord = { 0.0f, 1.0f };
    vertexData_[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };   // 左上
    vertexData_[1].texcoord = { 0.0f, 0.0f };
    vertexData_[2].position = { width, height, 0.0f, 1.0f };// 右下
    vertexData_[2].texcoord = { 1.0f, 1.0f };
    vertexData_[3].position = { width, 0.0f, 0.0f, 1.0f };  // 右上
    vertexData_[3].texcoord = { 1.0f, 0.0f };


    // 2. 行列計算
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 viewMatrix = Identity4x4();
    Matrix4x4 projectionMatrix = makeOrthographicmMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);

    Matrix4x4 wvpMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

    // 3. マテリアルへの書き込み
    materialData_->WVP = wvpMatrix;

}

void Sprite::Draw(DirectXCommon* dxCommon, D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle)
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
    commandList->SetGraphicsRootDescriptorTable(1, textureSrvHandle);

    // 5. 描画コマンド
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}