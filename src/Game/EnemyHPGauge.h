#pragma once
#include "Sprite.h"
#include <string>

class EnemyHPGauge {
public:
  void Initialize();

  void Update();

  void Draw();

  void SetHP(float hp, float halfHp);

private:
  Sprite *sprite_ = nullptr;

  std::string normalTexturePath_; // 通常ゲージ
  std::string lowTexturePath_;    // 瀕死ゲージ

  Vector2 position_{800.0f,
                    40.0f};     // ← 適当な初期位置。必要ならScene側でSetしてOK
  Vector2 size_{200.0f, 20.0f}; // ★ 最大HP時のゲージ幅
  Vector2 baseSize_{200.0f, 20.0f}; // ★ 変更前の基準サイズ

  float hpRatio_ = 1.0f;      // 0.0f ～ 1.0f
  float lowThreshold_ = 0.3f; // この値以下で瀕死色に切り替え
  bool isVisible_ = true;
};
