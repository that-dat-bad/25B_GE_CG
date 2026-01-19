#include "Object3d.h"
#include "Object3dCommon.h"
#include "TextureManager.h" 
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>
#include"Model.h"
#include"ModelManager.h"
#include"Camera.h"

void Object3d::Initialize(Object3dCommon* object3dCommon) {
	// メンバ変数に保存
	object3dCommon_ = object3dCommon;


	DirectXCommon* dxCommon = object3dCommon_->GetDirectXCommon();

	// 4. 座標変換行列の初期化

	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	transformationMatrixData_->WVP = Identity4x4();
	transformationMatrixData_->World = Identity4x4();



	// 5. 平行光源の初期化

	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));

	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;

	// 7. Transform初期値設定

	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	camera_ = object3dCommon_->GetDefaultCamera();
}

void Object3d::Update() {
	// 1. World行列を作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// 2. Camera行列を使ってWVPを作る
	Matrix4x4 worldViewProjectionMatrix;

	if (camera_) {
		// カメラがセットされていれば、そのカメラの ViewProjection 行列をもらう
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		// カメラが無い場合はとりあえずWorld行列だけ（または単位行列など）
		worldViewProjectionMatrix = worldMatrix;
	}

	// 3. 定数バッファに転送
	transformationMatrixData_->WVP = Transpose(worldViewProjectionMatrix);
	transformationMatrixData_->World = Transpose(worldMatrix);
}

void Object3d::Draw() {
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = object3dCommon_->GetDirectXCommon()->GetCommandList();

	// 座標変換行列CBufferの設定 (RootParameter Index: 1)
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	// 平行光源CBufferの設定 (RootParameter Index: 3)
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	commandList->SetGraphicsRootConstantBufferView(4, object3dCommon_->GetLightingSettingsResource()->GetGPUVirtualAddress());

	if (model_) {
		model_->Draw();
	}
}

void Object3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}
