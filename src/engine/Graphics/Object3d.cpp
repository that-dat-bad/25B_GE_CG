#include "Object3d.h"
#include "Object3dCommon.h"
#include "TextureManager.h" 
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>
#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"
#include "TDEngine.h"

Object3d* Object3d::Create() {
	Object3d* object3d = new Object3d();
	object3d->Initialize(TDEngine::GetObject3dCommon());
	return object3d;
}

void Object3d::Initialize(Object3dCommon* object3dCommon) {
	object3dCommon_ = object3dCommon;
	DirectXCommon* dxCommon = object3dCommon_->GetDirectXCommon();

	// 1. 座標変換行列の初期化
	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = Identity4x4();
	transformationMatrixData_->World = Identity4x4();

	// 2. 平行光源の初期化
	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;

	// ★3. マテリアル(色)の初期化
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白で初期化
	materialData_->enableLighting = false; // ライティング有効
	materialData_->uvTransform = Identity4x4();

	// 4. Transform初期値設定
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	camera_ = object3dCommon_->GetDefaultCamera();
}

void Object3d::Update() {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
}

void Object3d::Draw() {
	if (!model_) return; // モデルがなければ描画しない

	if (object3dCommon_) {
		object3dCommon_->SetupCommonState();
	}

	ID3D12GraphicsCommandList* commandList = object3dCommon_->GetDirectXCommon()->GetCommandList();

	// 1. 頂点バッファの設定 (Modelから取得)
	commandList->IASetVertexBuffers(0, 1, &model_->GetVertexBufferView());

	// 2. マテリアルCBufferの設定 (自分の色情報を使うため、modelではなくthisのバッファを使う)
	// RootParameter Index: 0
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// 3. 座標変換行列CBufferの設定 (RootParameter Index: 1)
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	// 4. テクスチャの設定 (RootParameter Index: 2)
	// Modelが持っているテクスチャインデックスを使う
	uint32_t texIndex = model_->GetTextureIndex();
	if (texIndex != 0) {
		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(texIndex);
		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);
	}

	// 5. 平行光源CBufferの設定 (RootParameter Index: 3)
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	// 6. 描画コマンド発行
	commandList->DrawInstanced(UINT(model_->GetVertexCount()), 1, 0, 0);
}

void Object3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}