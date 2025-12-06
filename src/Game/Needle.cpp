#include "Needle.h"
using namespace TDEngine;

void Needle::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position, const TDEngine::Vector3 rotate) {
	worldTransform_.translation_.x = 2.0f;
	worldTransform_.translation_.y = 2.0f;

	// nullポインタチェック
	assert(model);

	// 引数をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_ = rotate;
}

void Needle::Update() {
	// tを増加
	t += 0.01f;
	// サイズをイージング
	worldTransform_.scale_ = worldTransform_.EaseIn(t, worldTransform_.scale_, upScale_);
	// 行列を定数バッファに移動
	worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void Needle::Draw() {
	// モデルの描画
	model_->Draw(worldTransform_, *camera_);
}

TDEngine::Vector3 Needle::GetWorldPosition() { 
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos = worldTransform_.translation_;
	return worldPos;
}

OBB Needle::GetOBB() {
	OBB obb;

	// 中心点を定義する
	obb.center = GetWorldPosition();

	// サイズを定義する
	const TDEngine::Vector3& currentScale = worldTransform_.scale_;

	obb.size.x = width * currentScale.x;  // ワールドスケールを適用
	obb.size.y = height * currentScale.y; // ワールドスケールを適用
	obb.size.z = width * currentScale.z;  // ワールドスケールを適用

	// 3. 各軸の方向ベクトル (orientations) の計算
	const TDEngine::Matrix4x4& matWorld = worldTransform_.matWorld_;

	// 行列の第1列目を抽出し、正規化
	TDEngine::Vector3 xAxis = {matWorld.m[0][0], matWorld.m[0][1], matWorld.m[0][2]};
	obb.orientations[0] = worldTransform_.Normalize(xAxis);

	// Y軸（orientations[1]）: 行列の第2列目を抽出し、正規化
	TDEngine::Vector3 yAxis = {matWorld.m[1][0], matWorld.m[1][1], matWorld.m[1][2]};
	obb.orientations[1] = worldTransform_.Normalize(yAxis);

	// Z軸（orientations[2]）: 行列の第3列目を抽出し、正規化
	TDEngine::Vector3 zAxis = {matWorld.m[2][0], matWorld.m[2][1], matWorld.m[2][2]};
	obb.orientations[2] = worldTransform_.Normalize(zAxis);

	return obb;
}
