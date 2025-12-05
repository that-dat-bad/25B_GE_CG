#include "TitleLogo.h"

#include "../engine/Graphics/Camera.h"
#include "../engine/Graphics/Object3d.h"

#include <cassert>

using namespace MyMath;

void TitleLogo::Initialize(Object3d *obj, Camera *camera,
                           const Vector3 &position) {
  // nullポインタチェック
  assert(obj);

  // 引数をメンバ変数に記録
  obj_ = obj;
  camera_ = camera;

  // ワールドトランスフォームの初期化
  transform_.translate = position;
}

void TitleLogo::Update() {
  transform_.translate.y = sinf(theta_) * amplitude_;
  theta_ += static_cast<float>(M_PI) / 60.0f;
  transform_.translate.y += 2.0f;

  obj_->SetTranslate(transform_.translate);

  obj_->Update();

  // 行列を定数バッファに移動
  //transform_.UpdateWorldMatrix(transform_);
}

void TitleLogo::Draw() {
  // モデルの描画
  obj_->Draw();
}