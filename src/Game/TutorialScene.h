#pragma once
#include "AudioManager.h"
#include "Collision.h"
#include "Enemy.h"
#include "Fade.h"
#include "Player.h"
#include "Skydome.h"
#include "Sprite.h"

class TutorialScene {
public:
  ~TutorialScene();
  void Initialize();
  void Update();
  void Draw();
  bool IsFinished() const { return isFinished_; }

private:
  void UpdateFadeIn();
  void UpdateMain();
  void UpdateFadeOut();
  void CheckAllCollisions();

  enum class Phase { kFadeIn, kMain, kFadeOut };
  Phase phase_ = Phase::kMain;

  bool isCleared_ = false;
  bool isFinished_ = false;
  int changeTimer_ = 60;

  Fade *fade_ = nullptr;
  float duration_ = 0.5f;

  Sprite *rule_ = nullptr;
  Sprite *operation_ = nullptr;

  Player *player_ = nullptr;
  Enemy *enemy_ = nullptr;
  Skydome *skydome_ = nullptr;
  // BGM
  SoundData soundBgm_;
  IXAudio2SourceVoice *pBgmVoice_ = nullptr;
};