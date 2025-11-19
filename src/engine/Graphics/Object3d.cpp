#include "Object3d.h"
#include "Object3dCommon.h"
#include "TextureManager.h" 
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>
#include"Model.h"


void Object3d::Initialize(Object3dCommon* object3dCommon)
{
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
	cameraTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };
}

void Object3d::Update()
{
	// TransformからWorldMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// CameraTransformからCameraMatrixを作る
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);

	// CameraMatrixからViewMatrixを作る (逆行列)
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	//Matrix4x4 viewMatrix = MakeTranslateMatrix({ 0.0f, 0.0f, 10.0f });
	// ProjectionMatrixを作る
	Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);

	// WVP行列を作成して書き込む
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
}

void Object3d::Draw()
{
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = object3dCommon_->GetDirectXCommon()->GetCommandList();

	// 座標変換行列CBufferの設定 (RootParameter Index: 1)
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	// 平行光源CBufferの設定 (RootParameter Index: 3)
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	//// 描画コマンド発行
	//commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
	if (model_)
	{
		model_->Draw();
	}
}
