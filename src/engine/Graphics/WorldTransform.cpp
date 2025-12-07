#include "WorldTransform.h"
#include "DirectXCommon.h"
#include <cassert>

using namespace TDEngine;
using namespace MyMath;

void WorldTransform::Initialize() {
	// 定数バッファの生成
	constBuff_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataWorldTransform));

	// 定数バッファのマッピング
	HRESULT hr = constBuff_->Map(0, nullptr, reinterpret_cast<void**>(&constMap));
	assert(SUCCEEDED(hr));

	// 初期値設定
	scale_ = { 1.0f, 1.0f, 1.0f };
	rotation_ = { 0.0f, 0.0f, 0.0f };
	translation_ = { 0.0f, 0.0f, 0.0f };
	matWorld_ = Identity4x4();

	// 行列更新と転送
	UpdateMatrix();
}

void WorldTransform::UpdateMatrix() {
	// スケール、回転、平行移動行列の計算
	Matrix4x4 matScale = MakeScaleMatrix(scale_);
	Matrix4x4 matRot = MakeRotateMatrix(rotation_);
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);

	// ワールド行列の合成
	matWorld_ = Multiply(matScale, Multiply(matRot, matTrans));

	// 親があれば親の行列を掛ける
	if (parent_) {
		matWorld_ = Multiply(matWorld_, parent_->matWorld_);
	}

	// GPUへの転送
	TransferMatrix();
}

void WorldTransform::TransferMatrix() {
	if (constMap) {
		constMap->matWorld = matWorld_;
		// WVPの計算は描画時(Model::Draw)にCameraと合わせて行い、上書きされる想定
		// ここではとりあえず単位行列などを入れておいても良いが、matWorldだけ更新しておく
	}
}