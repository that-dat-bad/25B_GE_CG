#include "ChainBomb.h"
#include "Enemy.h"
#include "Player.h"
#include "TDEngine.h" // Inputなどが必要なら

using namespace MyMath;

ChainBomb::~ChainBomb() {
	if (object3d_) delete object3d_;
	if (deathParticle_) delete deathParticle_;
	AudioManager* audio = TDEngine::GetAudioManager();
	//audio->StopAllVoices();
	audio->SoundUnload(&bombSe_);

}

void ChainBomb::Initialize(const Vector3 &position) {
  std::string path = "./Resources/bomb/bomb.obj";
  Model::LoadFromOBJ(path);
  object3d_ = Object3d::Create();
  object3d_->SetModel(path);
  object3d_->SetTranslate(position);
  object3d_->SetScale({1.0f, 1.0f, 1.0f});

  object3d_->Update();

	explodeTimer_ = kExplodeFrame;

	AudioManager* audio = TDEngine::GetAudioManager();
	bombSe_ = audio->SoundLoadWave("Resources/Sound/bomb.wav");
}

void ChainBomb::Update() {
  // すでに破壊済み → 復活待ち
  if (isDestroy_) {
    reviveTimer_++;
    if (reviveTimer_ >= kReviveFrame) {
      Restore(); // 20秒後に復活
    }
    return;
  }

  // ★ リスポーン演出中
  if (isRespawning_) {
    respawnTimer_++;

    // 一応見た目更新だけはしておく
    if (object3d_) {
      object3d_->Update();
    }

    // 一定時間経過で演出終了
    if (respawnTimer_ >= kRespawnEffectFrame) {
      isRespawning_ = false;
      respawnTimer_ = 0;
    }

    return; // 演出中は爆発ロジックは動かさない
  }

  // --- ここから先は通常時の処理 ---

  if (object3d_)
    object3d_->Update();

  if (isExplode_ && !isDestroy_) {
    --explodeTimer_;
    if (deathParticle_)
      deathParticle_->Update();

    if (explodeTimer_ <= 0) {
      isDestroy_ = true;
      reviveTimer_ = 0; // 復活カウント開始
    }
  }
}


void ChainBomb::Draw() {
  if (isDestroy_)
    return;

  // ★ チカチカ用の可視フラグ
  bool visible = true;
  if (isRespawning_) {
    // 5フレームごとに ON/OFF 切り替え（いい感じに調整してOK）
    int blinkUnit = 5;
    visible = ((respawnTimer_ / blinkUnit) % 2 == 0);
  }

  if (!visible) {
    // チカチカで「消えてる状態」のときは何も描画しない
    return;
  }

  if (!isDestroy_) {
    if (isExplode_) {
      if (deathParticle_)
        deathParticle_->Draw();
    } else {
      if (object3d_)
        object3d_->Draw();
    }
  }
}


void ChainBomb::ExplodeAround(const std::vector<ChainBomb *> &allChainBombs,
                              float chainExplosionRadius) {
  // 自分が爆発中じゃない or もう壊れてるなら連鎖元にならない
  if (!isExplode_ || isDestroy_) {
    return;
  }

  Vector3 myPos = GetWorldPosition();
  float radiusSq = chainExplosionRadius * chainExplosionRadius;

  for (ChainBomb *other : allChainBombs) {
    if (!other)
      continue;
    if (other == this)
      continue;

    // すでに爆発してる or 壊れてるやつには連鎖しない
    if (other->IsExplode() || other->IsDestroy()) {
      continue;
    }

    Vector3 otherPos = other->GetWorldPosition();
    Vector3 diff = {otherPos.x - myPos.x, otherPos.y - myPos.y,
                    otherPos.z - myPos.z};
    float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

    if (distSq <= radiusSq) {
      other->Explode(); // ★ 爆発範囲内のchainbombも爆発
    }
  }
}


void ChainBomb::Explode() {
  if (isExplode_ || isDestroy_)
    return;

  isExplode_ = true;

  // エフェクト生成
  deathParticle_ = new EnemyDeathParticle();
  // Initializeの引数を調整
  deathParticle_->Initialize(GetWorldPosition());
}

void ChainBomb::OnCollision(const Player* player) {
  if (isExplode_ || isDestroy_)
    return;

	if (player->IsExplode()) {
		Explode();
		TDEngine::GetAudioManager()->SoundPlayWave(bombSe_, false, 1.0f);
	}
}

void ChainBomb::OnCollision(const Enemy *enemy) {
  // 敵との衝突ロジックがあれば記述
}

Vector3 ChainBomb::GetWorldPosition() const {
  if (object3d_)
    return object3d_->GetTranslate();
  return {0, 0, 0};
}

AABB ChainBomb::GetAABB() {
  Vector3 pos = GetWorldPosition();
  return {
      {pos.x - size_.x / 2.0f, pos.y - size_.y / 2.0f, pos.z - size_.z / 2.0f},
      {pos.x + size_.x / 2.0f, pos.y + size_.y / 2.0f, pos.z + size_.z / 2.0f}};
}

AABB ChainBomb::GetAABB(float playerSize) {
  // プレイヤーサイズなどを考慮した判定範囲が必要な場合
  Vector3 pos = GetWorldPosition();
  // 元コードでは引数のsizeを使っていたのでそれに合わせる
  float s = playerSize; // あるいは size_.x? 元コードは引数sizeを使用
  return {{pos.x - s / 2.0f, pos.y - s / 2.0f, pos.z - s / 2.0f},
          {pos.x + s / 2.0f, pos.y + s / 2.0f, pos.z + s / 2.0f}};
}

void ChainBomb::Restore() {
  isDestroy_ = false;
  isExplode_ = false;
  explodeTimer_ = kExplodeFrame;

  // モデルが消えている場合だけ再生成
  if (!object3d_) {
    std::string path = "./Resources/bomb/bomb.obj";
    Model::LoadFromOBJ(path);
    object3d_ = Object3d::Create();
    object3d_->SetModel(path);
  }

  // 見た目復活
  object3d_->SetScale({1.0f, 1.0f, 1.0f});
  object3d_->Update();

  // 爆発パーティクルは消しておく
  if (deathParticle_) {
    delete deathParticle_;
    deathParticle_ = nullptr;
  }

  // ★ リスポーン演出開始
  isRespawning_ = true;
  respawnTimer_ = 0;
}
