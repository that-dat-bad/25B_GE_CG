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
		glyphs_[i] = Sprite::Create(
		    texDigit_[0], {0.0f, 0.0f}, // 位置は後で LayoutGlyphs で決める
		    Vector4(1, 1, 1, 1));
	}

	// 中央に出すやつ
	Vector2 centerPos{640.0f, 300.0f};
	centerDigit_ = Sprite::Create(texDigit_[0], centerPos, {1, 1, 1, 1});

	// ここで大きさ・位置をまとめて設定
	LayoutGlyphs();

	// 初期表示更新
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

	// スプライト描画前処理
	Sprite::PreDraw();

	switch (phase_) {

	case Phase::kStartCountDown:
		DrawStartCountDownSprite(); // 3,2,1どーん

		break;

	case Phase::kActive:
		DrawTimeGlyphs(); // SS.ff 表示

		break;
	case Phase::kLast5Second:
		DrawLast5SecondSprite(); // 残り5秒どーん

		break;
	}

	// スプライト描画後処理
	Sprite::PostDraw();
}

void TimeLimit::UpdateGlyphTexturesFromTime(TimeLimit& self) {
	float t = std::max(self.timer_, 0.0f);

	// 1/100秒に変換（四捨五入）
	int centiSec = static_cast<int>(t * 100.0f + 0.5f);
	int seconds = centiSec / 100;  // 0~
	int fraction = centiSec % 100; // 0~99

	// 秒を 0~999 にクランプ（必要ならもっと増やせる）
	if (seconds > 999) {
		seconds = 999;
		fraction = 99;
	}

	// 秒を 3桁に分解
	int s100 = (seconds / 100) % 10; // 百の位
	int s10 = (seconds / 10) % 10;   // 十の位
	int s1 = seconds % 10;           // 一の位
	int f10 = (fraction / 10) % 10;  // 小数一桁目
	int f1 = fraction % 10;          // 小数二桁目

	int idx = 0;
	// [0] 秒100の位
	glyphs_[idx++]->SetTextureHandle(texDigit_[s100]);
	// [1] 秒10の位
	glyphs_[idx++]->SetTextureHandle(texDigit_[s10]);
	// [2] 秒1の位
	glyphs_[idx++]->SetTextureHandle(texDigit_[s1]);
	// [3] コロンを "." として使う
	glyphs_[idx++]->SetTextureHandle(texColon_); // ここは「.」画像でもOK
	// [4] 小数10の位
	glyphs_[idx++]->SetTextureHandle(texDigit_[f10]);
	// [5] 小数1の位
	glyphs_[idx++]->SetTextureHandle(texDigit_[f1]);
}

void TimeLimit::LayoutGlyphs() {
	// 全体の基準
	Vector2 basePos{100.0f, 50.0f};

	// 整数部(秒 3桁)の大きさ
	float mainWidth = 32.0f;
	float mainHeight = 64.0f;

	// 小数部( . + 小数2桁 )のスケール
	float fracScale = 0.7f;
	float fracWidth = mainWidth * fracScale;
	float fracHeight = mainHeight * fracScale;

	// ===== 整数部（秒 3桁） =====
	for (int i = 0; i < 3; ++i) { // 0,1,2
		float x = basePos.x + mainWidth * i;
		float y = basePos.y;
		glyphs_[i]->SetPosition({x, y});
		glyphs_[i]->SetSize({mainWidth, mainHeight});
	}

	// ===== 小数部（".", 小数1桁目, 小数2桁目） =====
	// "." は整数部のすぐ右
	float dotX = basePos.x + mainWidth * 3;
	float dotY = basePos.y + (mainHeight - fracHeight); // 下揃えっぽくする

	// [3] .
	glyphs_[3]->SetPosition({dotX, dotY});
	glyphs_[3]->SetSize({fracWidth, fracHeight});

	// [4] 小数1桁目
	float frac1X = dotX + fracWidth;
	float frac1Y = dotY;
	glyphs_[4]->SetPosition({frac1X, frac1Y});
	glyphs_[4]->SetSize({fracWidth, fracHeight});

	// [5] 小数2桁目
	float frac2X = frac1X + fracWidth;
	float frac2Y = dotY;
	glyphs_[5]->SetPosition({frac2X, frac2Y});
	glyphs_[5]->SetSize({fracWidth, fracHeight});
}

void TimeLimit::DrawStartCountDownSprite() {
	if (phase_ != Phase::kStartCountDown) {
		return;
	}

	int n = std::clamp((int)std::ceil(countDown_), 1, 3);
	centerDigit_->SetTextureHandle(texDigit_[n]);
	centerDigit_->SetSize({256, 256});
	centerDigit_->SetColor({1, 1, 1, 0.5f});
	centerDigit_->SetPosition({500, 200});
	centerDigit_->Draw();
}

void TimeLimit::DrawLast5SecondSprite() {
	if (phase_ != Phase::kLast5Second) {
		return;
	}

	int n = std::clamp((int)std::ceil(timer_), 1, 5);
	centerDigit_->SetTextureHandle(texDigit_[n]);
	centerDigit_->SetSize({512, 512});
	centerDigit_->SetColor({1, 1, 1, 0.35f});
	centerDigit_->SetPosition({400, 100});
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
	// スタートのカウントダウン
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
