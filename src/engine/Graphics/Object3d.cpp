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
	object3dCommon_ = object3dCommon;


	DirectXCommon* dxCommon = object3dCommon_->GetDirectXCommon();


	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	transformationMatrixData_->WVP = Identity4x4();
	transformationMatrixData_->World = Identity4x4();
	transformationMatrixData_->WorldInverseTranspose = Identity4x4();


	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	camera_ = object3dCommon_->GetDefaultCamera();
}

void Object3d::Update() {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	Matrix4x4 worldViewProjectionMatrix;

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WorldInverseTranspose = Transpose(Inverse(worldMatrix));
}

void Object3d::Draw() {
	ID3D12GraphicsCommandList* commandList = object3dCommon_->GetDirectXCommon()->GetCommandList();

	object3dCommon_->SetBlendMode(blendMode_);

	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	commandList->SetGraphicsRootConstantBufferView(3, object3dCommon_->GetDirectionalLightResource()->GetGPUVirtualAddress());

	commandList->SetGraphicsRootConstantBufferView(4, object3dCommon_->GetLightingSettingsResource()->GetGPUVirtualAddress());
	
	commandList->SetGraphicsRootConstantBufferView(5, object3dCommon_->GetPointLightResource()->GetGPUVirtualAddress());

	commandList->SetGraphicsRootConstantBufferView(6, object3dCommon_->GetSpotLightResource()->GetGPUVirtualAddress());

	if (model_) {
		model_->Draw();
	}
}

void Object3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}

