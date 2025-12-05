#include "BackGround.h"

#include "../engine/Graphics/Camera.h"
#include "../engine/Graphics/Object3d.h"

#include <cassert>

using namespace MyMath;

void BackGround::Initialize(Object3d *obj, Camera *camera,
                            const Vector3 &position) {
  // nullポインタチェック
  assert(obj);

  // 引数をメンバ変数に記録
  obj_ = obj;
  camera_ = camera;

  // ワールドトランスフォームの初期化
  transform_.translate = position;
}

void BackGround::Update() {

    obj_->Update();

  // 行列を定数バッファに移動
  // worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void BackGround::Draw() {
  // モデルの描画
  obj_->Draw();
}
