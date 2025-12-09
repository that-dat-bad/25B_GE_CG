#include "EnemyHPGauge.h"
#include "TDEngine.h"
#include "TextureManager.h"

#include <cstdio>
#include <algorithm>

void EnemyHPGauge::Initialize() {
  

  // テクスチャ読み込み＆保持
  normalTexturePath_ = "Resources/enemyHP.png";
  lowTexturePath_ = "Resources/enemyHPAlmostDie.png";
  TextureManager::LoadTexture(normalTexturePath_);
  TextureManager::LoadTexture(lowTexturePath_);

  // 基準サイズを保持
  baseSize_ = size_;

  // 最初は通常ゲージで生成
  sprite_ = Sprite::Create(normalTexturePath_, position_, {1, 1, 1, 1});
  sprite_->SetSize(size_);
}

void EnemyHPGauge::SetHP(float hp, float halfHp) {

  // -------- ゲージ色の切り替え --------
  if (hp <= halfHp - 10) {
    sprite_->SetTexture(lowTexturePath_); // 赤（瀕死）
  } else {
    sprite_->SetTexture(normalTexturePath_); // 黄（通常）
  }

  if (hp <= 0) {
    hp = 0;
  }

  sprite_->SetSize(Vector2{hp * 4.0f, 50.0f});
}


void EnemyHPGauge::Update() {
  if (!isVisible_ || !sprite_)
    return;

  sprite_->SetPosition(position_);
  sprite_->Update();
}

void EnemyHPGauge::Draw() {
  if (!isVisible_ || !sprite_)
    return;

  DirectXCommon *dxCommon = TDEngine::GetSpriteCommon()->GetDirectXCommon();
  sprite_->Draw(dxCommon);
}
