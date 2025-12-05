#pragma once

#include <cstdint>

class Sprite;
class SpriteCommon;
class DirectXCommon;

class Fade {
public:
  enum class Status {
    kNone,    // フェードなし
    kFadeIn,  // フェードイン中
    kFadeOut, // フェードアウト中
  };

  Status status_ = Status::kNone;

public:
  // 初期化
  void Initialize(SpriteCommon *spCom, DirectXCommon *dxCom);
  // 更新
  void Update();
  // 描画
  void Draw();

  // 演出開始
  void Start(Status status, float duration);
  // 演出終了
  void Stop();

public:
  bool IsActive() const { return isActive_; }
  bool IsFinished() const;

private:
  DirectXCommon *dxCom_ = nullptr;
  SpriteCommon *spCom_ = nullptr;
  Sprite *sprite_ = nullptr;
  uint32_t textureIndex_ = 0;

  bool isActive_ = false;

  float timer_ = 0.0f;
  float duration_ = 0.0f;
  static inline const float kDeltaTime = 1.0f / 60.0f;

  float startScale_ = 0.1f;
  float endScale_ = 1800.0f;

private:
  // フェードインの更新
  void UpdateFadeIn();
  // フェードアウトの更新
  void UpdateFadeOut();
};
