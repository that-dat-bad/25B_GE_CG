#include "Model.h"
#include "Object3d.h"

void Model::Initialize(ModelCommon* modelCommon)
{
	modelCommon_ = modelCommon;
	//モデル読み込み
	Object3d* object3d;
	//モデル読み込み
	modelData_ = object3d->LoadObjFile("Resources/Models", "model.obj");

	//vertexResource(vertexBuffer)を作る
	vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
	//VertexBufferViewの作成
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	//VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	//マテリアルリソースの作成
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = false;
	materialData_->uvTransform = Identity4x4();

	//.objの参照しているテクスチャファイルの読み込み
	TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
	//読み込んだテクスチャの番号を取得
	modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
}

