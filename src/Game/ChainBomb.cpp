#include "ChainBomb.h"
#include "Enemy.h"
#include "Player.h"

using namespace TDEngine;

/// <summary>
/// 初期化
/// </summary>
void ChainBomb::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position) {

	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// ワールドトランスフォーム更新
	worldTransform_.UpdateWorldMatrix(worldTransform_);

	explodeTimer_ = kExplodeFrame;

	// デスパーティクルのモデル
	modelParticle_ = Model::CreateFromOBJ("bombParticle", true);
}
/// <summary>
/// 更新
/// </summary>
void ChainBomb::Update() {

	// ワールドトランスフォーム更新
	worldTransform_.UpdateWorldMatrix(worldTransform_);

	if (isExplode_ && !isDestroy_) {
		--explodeTimer_;

		// ここで演出用の処理
		// デスパーティクルの更新
		deathParticle_->Update();

		// explodeTimer_が0に到達したら消滅
		if (explodeTimer_ <= 0) {
			isDestroy_ = true;
		}
	}
}
/// <summary>
/// 描画
/// </summary>
void ChainBomb::Draw() {
	if (isDestroy_) {
		return; // 完全に消滅したら描画しない
	}
	else {
		
		if (isExplode_)
		{
			deathParticle_->Draw();
		} else {
			model_->Draw(worldTransform_, *camera_);
		}
	}
}

void ChainBomb::ExplodeAround(const std::vector<ChainBomb*>& allChainBombs, float chainExplosionRadius) {
	// 爆発していないなら連鎖しない
	if (!isExplode_) {
		return;
	}

	// explodeTimer_が0になるまで連鎖しない
	if (explodeTimer_ > 0) {
		return;
	}

	// 自分のワールド座標を取得
	Vector3 originBombWorldPosition = GetWorldPosition();

	// すべてのボムをチェック
	for (ChainBomb* otherBomb : allChainBombs) {

		// 自分自身は対象外
		if (otherBomb == this) {
			continue;
		}

		// すでに破壊済みのボムも対象外
		if (otherBomb->isExplode_) {
			continue;
		}

		// 相手ボムのワールド座標
		Vector3 otherBombWorldPosition = otherBomb->GetWorldPosition();

		// 位置の差
		Vector3 positionDifference = {otherBombWorldPosition.x - originBombWorldPosition.x, otherBombWorldPosition.y - originBombWorldPosition.y, otherBombWorldPosition.z - originBombWorldPosition.z};

		// 距離の二乗を計算（平方根は取らない）
		float distanceSquared = positionDifference.x * positionDifference.x + positionDifference.y * positionDifference.y + positionDifference.z * positionDifference.z;

		// 連鎖爆発の範囲内なら爆発させる
		if (distanceSquared <= chainExplosionRadius * chainExplosionRadius) {
			otherBomb->Explode();
		}
	}
}

void ChainBomb::Explode() {
	if (isExplode_ || isDestroy_) {
		return;
	}

	isExplode_ = true;
	// デスパーティクルの初期化
	deathParticle_ = new EnemyDeathParticle();
	deathParticle_->Initialize(modelParticle_, camera_, worldTransform_.translation_);
}

// プレイヤーとの衝突応答
void ChainBomb::OnCollision(const Player* player) {
	if (player->IsExplode()) {

		Explode();

		(void)player;
	}
}
// プレイヤーとのAABB
AABB ChainBomb::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - size_.x / 2.0f, worldPos.y - size_.y / 2.0f, worldPos.z - size_.z / 2.0f};
	aabb.max = {worldPos.x + size_.x / 2.0f, worldPos.y + size_.y / 2.0f, worldPos.z + size_.z / 2.0f};

	return aabb;
}


// 敵との当たり判定
void ChainBomb::OnCollision(const Enemy* enemy) {
	if (!enemy->IsDead() && isExplode_) {

		(void)enemy;
	}
}
// 敵とのAABB
AABB ChainBomb::GetAABB(float size) {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - size / 2.0f, worldPos.y - size / 2.0f, worldPos.z - size / 2.0f};
	aabb.max = {worldPos.x + size / 2.0f, worldPos.y + size / 2.0f, worldPos.z + size / 2.0f};

	return aabb;
}

Vector3 ChainBomb::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}