#pragma once

#include "../engine/base/Math/MyMath.h"
#include "../engine/base/Math/Transform.h"

class Object3d;
class Camera;

class BackGround {
public:
  /// <summary>
  /// 初期化
  /// </summary>
  /// <param name="model">モデル</param>
  /// <param name="position">初期座標</param>

  void Initialize(Object3d *obj, Camera *camera,
                  const MyMath::Vector3 &position);

  /// <summary>
  /// 更新
  /// </summary>

  void Update();

  /// <summary>
  /// 描画
  /// </summary>

  void Draw();

private:
  // ワールド変換データ
  MyMath::Transform transform_;
  // モデル
  Object3d *obj_ = nullptr;

  // カメラ
  Camera *camera_ = nullptr;
};
