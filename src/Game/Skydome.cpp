#include <TDEngine.h>

#include "Skydome.h"
using namespace TDEngine;

void Skydome::Initialize(Model* model, Camera* camera) {
	// nullポインタチェック
	assert(model);

	// 引数をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
};

void Skydome::Update() { worldTransform_.UpdateWorldMatrix(worldTransform_); };
void Skydome::Draw() { model_->Draw(worldTransform_, *camera_); };