#include "Enemy.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "CameraManager.h"
#include "ModelManager.h"

void Enemy::Initialize(const MyMath::Vector3& position, const std::string& modelPath, float maxHealth) {
	position_ = position;
	maxHealth_ = maxHealth;
	health_ = maxHealth;
	isAlive_ = true;

	// モデルの読み込み（既にロード済みなら内部でスキップされる）
	ModelManager::GetInstance()->LoadModel(modelPath);

	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
	object3d_->SetModel(modelPath);

	// 位置を設定
	object3d_->SetTranslate(position_);

	// 地上ターゲット用のスケール調整
	object3d_->SetScale({ 3.0f, 3.0f, 3.0f });
}

void Enemy::Update() {
	if (!isAlive_) { return; }

	object3d_->Update();
}

void Enemy::Draw() {
	if (!isAlive_) { return; }
	if (!object3d_) { return; }

	object3d_->Draw();
}

void Enemy::TakeDamage(float damage) {
	if (!isAlive_) { return; }

	health_ -= damage;
	if (health_ <= 0.0f) {
		health_ = 0.0f;
		isAlive_ = false;
	}
}
