#include "TimeLimit.h"
#include "TDEngine.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

TimeLimit::~TimeLimit() {
	for (auto s : glyphs_) if (s) delete s;
	if (centerDigit_) delete centerDigit_;
}

void TimeLimit::Initialize() {
  phase_ = Phase::kStartCountDown;
  timer_ = kTimeLimit;
  countDown_ = 3.0f;

  // パス準備
  for (int i = 0; i < 10; ++i) {
    std::string path = "Resources/number/" + std::to_string(i) + ".png";
    digitTexturePaths_.push_back(path);
    TextureManager::LoadTexture(path);
  }
  colonTexturePath_ = "Resources/number/dot.png";
  TextureManager::LoadTexture(colonTexturePath_);
  // Sprite生成 (初期テクスチャは0番)
  for (int i = 0; i < kMaxGlyphs; ++i) {
    glyphs_[i] = Sprite::Create(digitTexturePaths_[0], {0, 0}, {1, 1, 1, 1});
  }
  centerDigit_ = Sprite::Create(digitTexturePaths_[0], {640, 300}, {1, 1, 1, 1},
                                {0.5f, 0.5f});

  UpdateGlyphTexturesFromTime();
  LayoutGlyphs({32, 64});
}

void TimeLimit::Update() {
	float dt = 1.0f / 60.0f;
	switch (phase_) {
	case Phase::kStartCountDown:
		countDown_ -= dt;
		if (countDown_ <= 0.0f) phase_ = Phase::kActive;
		break;
	case Phase::kActive:
		timer_ -= dt;
		UpdateGlyphTexturesFromTime();
        LayoutGlyphs({32, 64});
		if (timer_ <= 5.0f) phase_ = Phase::kLast5Second;
		break;
	case Phase::kLast5Second:
		timer_ -= dt;
		UpdateGlyphTexturesFromTime();
        LayoutGlyphs({256, 256});
		if (timer_ <= 0.0f) phase_ = Phase::kTimeUp;
		break;
	case Phase::kTimeUp:
		break;
	}

	// Sprite更新
	for (auto s : glyphs_) if (s) s->Update();
	if (centerDigit_) centerDigit_->Update();
}

void TimeLimit::Draw() {
	if (phase_ == Phase::kTimeUp) return;

	// Sprite描画準備はScene側で行っている前提だが、ここでも描画コマンド発行
	DirectXCommon* dxCommon = TDEngine::GetSpriteCommon()->GetDirectXCommon();

	if (phase_ == Phase::kStartCountDown) {
		int n = std::clamp((int)std::ceil(countDown_), 1, 3);
		// テクスチャ差し替え
		centerDigit_->SetTexture(digitTexturePaths_[n]);
		centerDigit_->SetSize({ 256, 256 });
		centerDigit_->Draw(dxCommon);
	}
	else if (phase_ == Phase::kLast5Second) {
		// 背景のデカ文字
		int n = std::clamp((int)std::ceil(timer_), 1, 5);
		centerDigit_->SetTexture(digitTexturePaths_[n]);
		centerDigit_->SetSize({ 256, 256 });
		centerDigit_->SetColor({ 1, 1, 1, 0.35f });
		centerDigit_->Draw(dxCommon);

		//// 前面タイマー
		//for (auto s : glyphs_) if (s) s->Draw(dxCommon);
	}
	else if (phase_ == Phase::kActive) {
		for (auto s : glyphs_) if (s) s->Draw(dxCommon);
	}
}

void TimeLimit::UpdateGlyphTexturesFromTime() {
	float t = (std::max)(timer_, 0.0f);
	int centiSec = static_cast<int>(t * 100.0f + 0.5f);
	int seconds = centiSec / 100;
	int fraction = centiSec % 100;

	if (seconds > 999) seconds = 999;

	int digits[6];
	digits[0] = (seconds / 100) % 10;
	digits[1] = (seconds / 10) % 10;
	digits[2] = seconds % 10;
	digits[3] = -1; // colon
	digits[4] = (fraction / 10) % 10;
	digits[5] = fraction % 10;

	for (int i = 0; i < kMaxGlyphs; ++i) {
		if (digits[i] == -1) {
			glyphs_[i]->SetTexture(colonTexturePath_);
		}
		else {
			glyphs_[i]->SetTexture(digitTexturePaths_[digits[i]]);
		}
	}
}

void TimeLimit::LayoutGlyphs(Vector2 size) {

Vector2 basePos{ 100.0f, 50.0f };
    float mainW = size.x, mainH = size.y;
	float fracScale = 0.7f;

	// 整数部
	for (int i = 0; i < 3; ++i) {
		glyphs_[i]->SetPosition({ basePos.x + mainW * i, basePos.y });
		glyphs_[i]->SetSize({ mainW, mainH });
	}
	// ドット
	float dotX = basePos.x + mainW * 3;
	float dotY = basePos.y + (mainH - mainH * fracScale);
	glyphs_[3]->SetPosition({ dotX, dotY });
	glyphs_[3]->SetSize({ mainW * fracScale, mainH * fracScale });

	// 小数部
	for (int i = 4; i < 6; ++i) {
		float x = dotX + (mainW * fracScale) * (i - 3);
		glyphs_[i]->SetPosition({ x, dotY });
		glyphs_[i]->SetSize({ mainW * fracScale, mainH * fracScale });
	}
}