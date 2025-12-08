#include "WorldTransform.h"
#include "DirectXCommon.h"
#include <cassert>

using namespace MyMath;

void WorldTransform::Initialize() {
	// 定数バッファの生成
	constBuff_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataWorldTransform));

	// 定数バッファのマッピング
	// ※一度マップしたら、Unmapせずに書き込み続ける方式（KamataEngine仕様）
	HRESULT hr = constBuff_->Map(0, nullptr, reinterpret_cast<void**>(&constMap));
	assert(SUCCEEDED(hr));

	// 初期値で行列更新・転送
	scale = { 1.0f, 1.0f, 1.0f };
	rotation = { 0.0f, 0.0f, 0.0f };
	translation = { 0.0f, 0.0f, 0.0f };
	matWorld = Identity4x4();
	TransferMatrix();
}

void WorldTransform::UpdateMatrix() {
	// スケール、回転、平行移動行列の計算
	Matrix4x4 matScale = MakeScaleMatrix(scale);
	Matrix4x4 matRot = MakeRotateMatrix(rotation); // Euler角(XYZ)から回転行列を作成する関数を使用
	Matrix4x4 matTrans = MakeTranslateMatrix(translation);

	// ワールド行列の合成
	matWorld = Multiply(matScale, Multiply(matRot, matTrans));

	// 親があれば親の行列を掛ける
	if (parent_) {
		matWorld = Multiply(matWorld, parent_->matWorld);
	}

	// GPUへの転送
	TransferMatrix();
}

void WorldTransform::TransferMatrix() {
	if (constMap) {
		constMap->matWorld = matWorld;
	}
}