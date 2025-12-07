#include "ChainBomb.h"
#include "Enemy.h"
#include "Player.h"

using namespace TDEngine;

void ChainBomb::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position) {
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation = position;

	// ワールドトランスフォーム更新
	worldTransform_.UpdateMatrix();

	explodeTimer_ = kExplodeFrame;

	// デスパーティクルのモデル
	modelParticle_ = TDEngine::Model::CreateFromOBJ("bombParticle", true);
}

void ChainBomb::Update() {
	// ワールドトランスフォーム更新
	worldTransform_.UpdateMatrix();

	if (isExplode_ && !isDestroy_) {
		--explodeTimer_;
		deathParticle_->Update();
		if (explodeTimer_ <= 0) {
			isDestroy_ = true;
		}
	}
}

void ChainBomb::Draw() {
	if (isDestroy_) {
		return;
	}
	else {
		if (isExplode_) {
			deathParticle_->Draw();
		}
		else {
			model_->Draw(worldTransform_, *camera_);
		}
	}
}

void ChainBomb::ExplodeAround(const std::vector<ChainBomb*>& allChainBombs, float chainExplosionRadius) {
	if (!isExplode_) return;
	if (explodeTimer_ > 0) return;

	Vector3 originBombWorldPosition = GetWorldPosition();

	for (ChainBomb* otherBomb : allChainBombs) {
		if (otherBomb == this) continue;
		if (otherBomb->isExplode_) continue;

		Vector3 otherBombWorldPosition = otherBomb->GetWorldPosition();
		Vector3 positionDifference = { otherBombWorldPosition.x - originBombWorldPosition.x, otherBombWorldPosition.y - originBombWorldPosition.y, otherBombWorldPosition.z - originBombWorldPosition.z };
		float distanceSquared = positionDifference.x * positionDifference.x + positionDifference.y * positionDifference.y + positionDifference.z * positionDifference.z;

		if (distanceSquared <= chainExplosionRadius * chainExplosionRadius) {
			otherBomb->Explode();
		}
	}
}

void ChainBomb::Explode() {
	if (isExplode_ || isDestroy_) return;

	isExplode_ = true;
	deathParticle_ = new EnemyDeathParticle();
	deathParticle_->Initialize(modelParticle_, camera_, worldTransform_.translation); // translation_ -> translation
}

void ChainBomb::OnCollision(const Player* player) {
	if (player->IsExplode()) {
		Explode();
		(void)player;
	}
}

AABB ChainBomb::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - size_.x / 2.0f, worldPos.y - size_.y / 2.0f, worldPos.z - size_.z / 2.0f };
	aabb.max = { worldPos.x + size_.x / 2.0f, worldPos.y + size_.y / 2.0f, worldPos.z + size_.z / 2.0f };
	return aabb;
}

void ChainBomb::OnCollision(const Enemy* enemy) {
	if (!enemy->IsDead() && isExplode_) {
		(void)enemy;
	}
}

AABB ChainBomb::GetAABB(float size) {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - size / 2.0f, worldPos.y - size / 2.0f, worldPos.z - size / 2.0f };
	aabb.max = { worldPos.x + size / 2.0f, worldPos.y + size / 2.0f, worldPos.z + size / 2.0f };
	return aabb;
}

Vector3 ChainBomb::GetWorldPosition() {
	Vector3 worldPos;
	// matWorld_ -> matWorld
	worldPos.x = worldTransform_.matWorld.m[3][0];
	worldPos.y = worldTransform_.matWorld.m[3][1];
	worldPos.z = worldTransform_.matWorld.m[3][2];
	return worldPos;
}