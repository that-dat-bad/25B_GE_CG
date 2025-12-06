#define NOMINMAX
#include "TimeLimit.h"
#include <algorithm>

using namespace TDEngine;

void TimeLimit::Initialize() {

	phase_ = Phase::kStartCountDown;
	timer_ = kTimeLimit;
	countDown_ = kCountDown;

	for (int i = 0; i < 10; ++i) {
		std::string path = "./Resources/number/" + std::to_string(i) + ".png";
		texDigit_[i] = TextureManager::Load(path);
	}
	texColon_ = TextureManager::Load("./Resources/number/dot.png");

	// --- glyph用Sprite生成 ---
	for (int i = 0; i < kMaxGlyphs; ++i) {
		// ハンドル指定のCreateを使う
		glyphs_[i] = Sprite::Create(
			texDigit_[0], { 0.0f, 0.0f },
			Vector4(1, 1, 1, 1));
	}

	Vector2 centerPos{ 640.0f, 300.0f };
	centerDigit_ = Sprite::Create(texDigit_[0], centerPos, { 1, 1, 1, 1 });

	LayoutGlyphs();
	UpdateGlyphTexturesFromTime(*this);
}

void TimeLimit::Update() {
	switch (phase_) {
	case Phase::kStartCountDown:
		UpdateStartCountDown();
		break;
	case Phase::kActive:
		UpdateActive();
		break;
	case Phase::kLast5Second:
		UpdateLast5Second();
		break;
	case Phase::kTimeUp:
		break;
	}
}

void TimeLimit::Draw() {
	if (phase_ == Phase::kTimeUp) {
		return;
	}

	// Model::PreDraw(); // 不要

	switch (phase_) {
	case Phase::kStartCountDown:
		DrawStartCountDownSprite();
		break;
	case Phase::kActive:
		DrawTimeGlyphs();
		break;
	case Phase::kLast5Second:
		DrawLast5SecondSprite();
		break;
	}

	// Model::PostDraw(); // 不要
}

void TimeLimit::UpdateGlyphTexturesFromTime(TimeLimit& self) {
	float t = std::max(self.timer_, 0.0f);

	int centiSec = static_cast<int>(t * 100.0f + 0.5f);
	int seconds = centiSec / 100;
	int fraction = centiSec % 100;

	if (seconds > 999) {
		seconds = 999;
		fraction = 99;
	}

	int s100 = (seconds / 100) % 10;
	int s10 = (seconds / 10) % 10;
	int s1 = seconds % 10;
	int f10 = (fraction / 10) % 10;
	int f1 = fraction % 10;

	int idx = 0;
	// ★追加した SetTextureHandle を使う
	glyphs_[idx++]->SetTextureHandle(texDigit_[s100]);
	glyphs_[idx++]->SetTextureHandle(texDigit_[s10]);
	glyphs_[idx++]->SetTextureHandle(texDigit_[s1]);
	glyphs_[idx++]->SetTextureHandle(texColon_);
	glyphs_[idx++]->SetTextureHandle(texDigit_[f10]);
	glyphs_[idx++]->SetTextureHandle(texDigit_[f1]);
}

void TimeLimit::LayoutGlyphs() {
	Vector2 basePos{ 100.0f, 50.0f };
	float mainWidth = 32.0f;
	float mainHeight = 64.0f;
	float fracScale = 0.7f;
	float fracWidth = mainWidth * fracScale;
	float fracHeight = mainHeight * fracScale;

	for (int i = 0; i < 3; ++i) {
		float x = basePos.x + mainWidth * i;
		float y = basePos.y;
		glyphs_[i]->SetPosition({ x, y });
		glyphs_[i]->SetSize({ mainWidth, mainHeight });
	}

	float dotX = basePos.x + mainWidth * 3;
	float dotY = basePos.y + (mainHeight - fracHeight);

	glyphs_[3]->SetPosition({ dotX, dotY });
	glyphs_[3]->SetSize({ fracWidth, fracHeight });

	float frac1X = dotX + fracWidth;
	float frac1Y = dotY;
	glyphs_[4]->SetPosition({ frac1X, frac1Y });
	glyphs_[4]->SetSize({ fracWidth, fracHeight });

	float frac2X = frac1X + fracWidth;
	float frac2Y = dotY;
	glyphs_[5]->SetPosition({ frac2X, frac2Y });
	glyphs_[5]->SetSize({ fracWidth, fracHeight });
}

void TimeLimit::DrawStartCountDownSprite() {
	if (phase_ != Phase::kStartCountDown) {
		return;
	}

	int n = std::clamp((int)std::ceil(countDown_), 1, 3);
	centerDigit_->SetTextureHandle(texDigit_[n]);
	centerDigit_->SetSize({ 256, 256 });
	centerDigit_->SetColor({ 1, 1, 1, 0.5f });
	centerDigit_->SetPosition({ 500, 200 });
	centerDigit_->Draw();
}

void TimeLimit::DrawLast5SecondSprite() {
	if (phase_ != Phase::kLast5Second) {
		return;
	}

	int n = std::clamp((int)std::ceil(timer_), 1, 5);
	centerDigit_->SetTextureHandle(texDigit_[n]);
	centerDigit_->SetSize({ 512, 512 });
	centerDigit_->SetColor({ 1, 1, 1, 0.35f });
	centerDigit_->SetPosition({ 400, 100 });
	centerDigit_->Draw();
}

void TimeLimit::DrawTimeGlyphs() {
	if (phase_ != Phase::kActive && phase_ != Phase::kLast5Second) {
		return;
	}

	for (int i = 0; i < kMaxGlyphs; ++i) {
		if (glyphs_[i]) {
			glyphs_[i]->Draw();
		}
	}
}

void TimeLimit::UpdateStartCountDown() {
	countDown_ -= kDeltaTime;
	if (countDown_ <= 0.0f) {
		phase_ = Phase::kActive;
	}
}

void TimeLimit::UpdateActive() {
	timer_ -= kDeltaTime;
	UpdateGlyphTexturesFromTime(*this);
	if (timer_ <= 5.0f) {
		phase_ = Phase::kLast5Second;
	}
}

void TimeLimit::UpdateLast5Second() {
	timer_ -= kDeltaTime;
	if (timer_ <= 0.0f) {
		phase_ = Phase::kTimeUp;
	}
}
TimeLimit::~TimeLimit() {
	for (auto* sprite : glyphs_) {
		if (sprite) delete sprite;
	}
	if (centerDigit_) delete centerDigit_;
}