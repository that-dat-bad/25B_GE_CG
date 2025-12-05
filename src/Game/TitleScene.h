#pragma once
#include "BackGround.h"
#include "Fade.h"
#include "TitleLogo.h"

#include "../engine/base/Math/MyMath.h"

class Camera;
class Model;
class SpriteCommon;
class DirectXCommon;
class Object3dCommon;
class Object3d;
class Input;

class TitleScene {
public:
  enum class Select {
    kNone,     // 未選択
    kTutorial, // チュートリアル
    kGame,     // ゲームプレイ
  };

  // 選択
  Select select_ = Select::kNone;

  enum class Phase {
    kFadeIn,  // フェードイン
    kMain,    // メイン
    kFadeOut, // フェードアウト
  };

  Phase phase_ = Phase::kMain;

public:
  /// <summary>
  /// 初期化
  /// </summary>
  void Initialize(Object3dCommon *objCom, SpriteCommon *spCom,
                  DirectXCommon *dxCom, Input* input);
  /// <summary>
  /// 更新
  /// </summary>
  void Update();
  /// <summary>
  /// 描画
  /// </summary>
  void Draw();

public:
  // 選択しているもの
  Select GetSelect() const { return select_; }

  // シーン終了フラグのGetter
  bool IsFinished() const { return isFinished_; }

private:
  Object3dCommon *objCom_ = nullptr;
  Input *input_ = nullptr;

  // フェード
  Fade fade_;
  float duration_ = 0.5f;

  // シーン終了フラグ
  bool isFinished_ = false;

  // カメラ
  Camera *camera_;

  // 背景
  BackGround *backGround_ = nullptr;
  // 背景の位置
  MyMath::Vector3 pos = {0.0f, 0.0f, 0.0f};
  // 背景のモデル
  Model *modelBackground_ = nullptr;
  Object3d *objBackground_ = nullptr;
  // タイトルロゴ
  TitleLogo *logo_ = nullptr;
  Object3d *objLogo_ = nullptr;
  // タイトルロゴのモデル
  Model *modelLogo_ = nullptr;

private:
  // フェードインの更新
  void UpdateFadeIn();
  // メインの更新
  void UpdateMain();
  // フェードアウトの更新
  void UpdateFadeOut();

  /// <summary>
  /// 選択
  /// </summary>
  void UpdateSelect();
};
