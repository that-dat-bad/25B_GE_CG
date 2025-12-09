#include "Fade.h"
#include "TDEngine.h"
#include "TextureManager.h"
#include <algorithm>

Fade::~Fade() {
  if (sprite_)
    delete sprite_;
}

void Fade::Initialize() {
  TextureManager::LoadTexture("./Resources/fade.png");

  // スプライト生成
  sprite_ = Sprite::Create("./Resources/fade.png", {640.0f, 360.0f},
                           {0, 0, 0, 1}, {0.5f, 0.5f});
  sprite_->SetSize({0.0f, 0.0f});
  isActive_ = false;
}

void Fade::Update() {
  switch (status_) {
  case Status::kFadeIn:
    UpdateFadeIn();
    break;
  case Status::kFadeOut:
    UpdateFadeOut();
    break;
  }
  if (sprite_)
    sprite_->Update();
}

void Fade::Draw() {
  if (status_ == Status::kNone || !sprite_)
    return;
  sprite_->Draw(TDEngine::GetSpriteCommon()->GetDirectXCommon());
}

void Fade::UpdateFadeIn() {
  timer_ += 1.0f / 60.0f;
  float t = 1.0f - std::clamp(timer_ / duration_, 0.0f, 1.0f);
  float scale = (1.0f - t) * startScale_ + t * endScale_;
  sprite_->SetSize({scale, scale});

  if (t >= 1.0f)
    isActive_ = false;
}

void Fade::UpdateFadeOut() {
  timer_ += 1.0f / 60.0f;
  float t = std::clamp(timer_ / duration_, 0.0f, 1.0f);
  float scale = (1.0f - t) * startScale_ + t * endScale_;
  sprite_->SetSize({scale, scale});

  if (t >= 1.0f)
    isActive_ = false;
}

void Fade::Start(Status status, float duration) {
  isActive_ = true;
  timer_ = 0.0f;
  duration_ = duration;
  status_ = status;
  if (sprite_) {
    sprite_->SetPosition({640.0f, 360.0f});
    sprite_->SetAnchorPoint({0.5f, 0.5f});
  }
}

void Fade::Stop() { status_ = Status::kNone; }

bool Fade::IsFinished() const {
  if (status_ == Status::kFadeIn || status_ == Status::kFadeOut) {
    return timer_ >= duration_;
  }
  return true;
}