#include "Fade.h"
#include "../engine/Graphics/DirectXCommon.h"
#include "../engine/Graphics/Sprite.h"
#include "../engine/Graphics/SpriteCommon.h"
#include "../engine/Graphics/TextureManager.h"
#include "../engine/base/Math/MyMath.h"
#include <algorithm>

using namespace MyMath;

void Fade::Initialize(SpriteCommon *spCom, DirectXCommon *dxCom) {

  spCom_ = spCom;
  dxCom_ = dxCom;

  std::string path = "assets/textures/fade.png";

  TextureManager::GetInstance()->LoadTexture(path);
  // テクスチャインデックスを取得
  textureIndex_ =
      TextureManager::GetInstance()->GetTextureIndexByFilePath(path);

  sprite_ = new Sprite();
  sprite_->Initialize(spCom, dxCom, path);

  /*sprite_ = Sprite::Create(textureIndex_, Vector2(640.0f, 360.0f),
                           Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  sprite_->SetSize({0.0f, 0.0f});*/

  isActive_ = false;
}

void Fade::Update() {

  sprite_->Update();

  switch (status_) {
  case Status::kNone:

    break;
  case Status::kFadeIn:

    UpdateFadeIn();

    break;
  case Status::kFadeOut:

    UpdateFadeOut();

    break;
  }
}

void Fade::Draw() {

  if (status_ == Status::kNone) {
    return;
  }

  spCom_->SetupCommonState();

  D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle =
      TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_);

  sprite_->Draw(dxCom_, textureSrvHandle);
}

// フェードインの更新
void Fade::UpdateFadeIn() {
  timer_ += kDeltaTime;
  float t = 1.0f - std::clamp(timer_ / duration_, 0.0f, 1.0f);

  // スケールを補間
  float scale = (1.0f - t) * startScale_ + t * endScale_;
  sprite_->SetSize({scale, scale});

  if (t >= 1.0f) {
    isActive_ = false;
  }
}

// フェードアウトの更新
void Fade::UpdateFadeOut() {
  timer_ += kDeltaTime;
  float t = std::clamp(timer_ / duration_, 0.0f, 1.0f);

  // スケールを補間
  float scale = (1.0f - t) * startScale_ + t * endScale_;
  sprite_->SetSize({scale, scale});

  if (t >= 1.0f) {
    isActive_ = false;
  }
}

void Fade::Start(Status status, float duration) {
  isActive_ = true;

  timer_ = 0.0f;
  duration_ = duration;
  status_ = status;

  sprite_->SetPosition(Vector2(640.0f, 360.0f));
  sprite_->SetAnchorPoint(Vector2{0.5f, 0.5f});
}
// 演出終了
void Fade::Stop() { status_ = Status::kNone; }

bool Fade::IsFinished() const {

  switch (status_) {
  case Status::kFadeIn:
  case Status::kFadeOut:
    if (timer_ >= duration_) {
      return true;
    } else {
      return false;
    }
  }

  return true;
}
