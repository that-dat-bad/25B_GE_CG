#include "Fade.h"
#include <algorithm>

using namespace TDEngine;

void Fade::Initialize() {

	texture_ = TextureManager::Load("./Resources/fade.png");

	sprite_ = Sprite::Create(texture_, Vector2(640.0f, 360.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	sprite_->SetSize({0.0f, 0.0f});

	isActive_ = false;
}

void Fade::Update() {

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

	// スプライト描画前処理
	Sprite::PreDraw();

	sprite_->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();
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
