#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

#include "../engine/base/Math/MyMath.h"
#include "../engine/base/Math/Transform.h"

class Object3d;
class Camera;

/// <summary>
/// タイトルシーン
/// </summary>

class TitleLogo {
private:
  // ワールド変換データ
  MyMath::Transform transform_;

  // モデル
  Object3d *obj_ = nullptr;

  // カメラ
  Camera *camera_ = nullptr;

  // タイトルロゴを動かす用の変数
  float amplitude_ = 0.5f;
  float theta_ = 0.0f;

public:
  void Initialize(Object3d *obj, Camera *camera,
                  const MyMath::Vector3 &position);
  void Update();
  void Draw();
};