#include "ChainBomb.h"
#include "Player.h"
#include "Enemy.h"
#include "TDEngine.h" // Inputなどが必要なら

using namespace MyMath;

ChainBomb::~ChainBomb() {
	if (object3d_) delete object3d_;
	if (deathParticle_) delete deathParticle_;
}

void ChainBomb::Initialize(const Vector3& position) {
	object3d_ = Object3d::Create();
	object3d_->SetModel("bomb.obj");
	object3d_->SetTranslate(position);
	object3d_->SetScale({ 1.0f, 1.0f, 1.0f });

	explodeTimer_ = kExplodeFrame;
}

void ChainBomb::Update() {
	if (object3d_) object3d_->Update();

	if (isExplode_ && !isDestroy_) {
		--explodeTimer_;
		if (deathParticle_) deathParticle_->Update();

		if (explodeTimer_ <= 0) {
			isDestroy_ = true;
		}
	}
}

void ChainBomb::Draw() {
	if (isDestroy_) return;

	if (isExplode_) {
		if (deathParticle_) deathParticle_->Draw();
	}
	else {
		if (object3d_) object3d_->Draw();
	}
}

void ChainBomb::ExplodeAround(const std::vector<ChainBomb*>& allChainBombs, float chainExplosionRadius) {
	if (!isExplode_) return;
	if (explodeTimer_ > 0) return; // 元コードのロジック維持(0になるまで連鎖しない?)

	Vector3 myPos = GetWorldPosition();

	for (ChainBomb* other : allChainBombs) {
		if (other == this || other->IsExplode()) continue;

		Vector3 otherPos = other->GetWorldPosition();
		Vector3 diff = { otherPos.x - myPos.x, otherPos.y - myPos.y, otherPos.z - myPos.z };
		float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (distSq <= chainExplosionRadius * chainExplosionRadius) {
			other->Explode();
		}
	}
}

void ChainBomb::Explode() {
	if (isExplode_ || isDestroy_) return;

	isExplode_ = true;

	// エフェクト生成
	deathParticle_ = new EnemyDeathParticle();
	// Initializeの引数を調整
	deathParticle_->Initialize(GetWorldPosition());
}

void ChainBomb::OnCollision(const Player* player) {
	if (player->IsExplode()) {
		Explode();
	}
}

void ChainBomb::OnCollision(const Enemy* enemy) {
	// 敵との衝突ロジックがあれば記述
}

Vector3 ChainBomb::GetWorldPosition() const {
	if (object3d_) return object3d_->GetTranslate();
	return { 0,0,0 };
}

AABB ChainBomb::GetAABB() {
	Vector3 pos = GetWorldPosition();
	return {
		{ pos.x - size_.x / 2.0f, pos.y - size_.y / 2.0f, pos.z - size_.z / 2.0f },
		{ pos.x + size_.x / 2.0f, pos.y + size_.y / 2.0f, pos.z + size_.z / 2.0f }
	};
}

AABB ChainBomb::GetAABB(float playerSize) {
	// プレイヤーサイズなどを考慮した判定範囲が必要な場合
	Vector3 pos = GetWorldPosition();
	// 元コードでは引数のsizeを使っていたのでそれに合わせる
	float s = playerSize; // あるいは size_.x? 元コードは引数sizeを使用
	return {
		{ pos.x - s / 2.0f, pos.y - s / 2.0f, pos.z - s / 2.0f },
		{ pos.x + s / 2.0f, pos.y + s / 2.0f, pos.z + s / 2.0f }
	};
}