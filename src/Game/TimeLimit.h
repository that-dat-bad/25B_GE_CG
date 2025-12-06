#pragma once

#include <TDEngine.h>

class TimeLimit {
public:
	~TimeLimit();

	void Initialize();
	void Update();
	void Draw();

	bool IsStartCountDown() const { return phase_ == Phase::kStartCountDown; }
	bool IsTImeUp() const { return phase_ == Phase::kTimeUp; }

public:
	enum class Phase {
		kStartCountDown, // スタートのカウントダウン
		kActive,         // ゲームの時間制限
		kLast5Second,    // 終了5秒前
		kTimeUp,         // 終了
	};

	Phase phase_ = Phase::kStartCountDown;

public:
	// 秒換算
	static inline const float kDeltaTime = 1.0f / 60.0f;
	// 制限時間
	float timer_ = 0.0f;
	// 上限（3分）
	static inline const float kTimeLimit = 10.0f;

	// スタートのカウントダウン
	static inline const float kCountDown = 3.0f;
	// カウントダウン
	float countDown_ = 0.0f;

	// 0～9 + コロン
	uint32_t texDigit_[10] = {};
	uint32_t texColon_ = 0;

	// 最大表示 [M][(M2)][ : ][S1][S2] → 最大5文字
	static const int kMaxGlyphs = 6;
	std::array<TDEngine::Sprite*, kMaxGlyphs> glyphs_{};

	TDEngine::Sprite* centerDigit_ = nullptr;

	// 終了フラグ
	bool isFinished_ = false;

private:
	void UpdateStartCountDown();
	void UpdateActive();
	void UpdateLast5Second();

	void LayoutGlyphs();             // 追加: 桁の位置と大きさ決め
	void DrawStartCountDownSprite(); // 追加: 3,2,1描画
	void DrawLast5SecondSprite();    // 追加: 残り5秒カウント描画
	void DrawTimeGlyphs();

	void UpdateGlyphTexturesFromTime(TimeLimit& timeLimit);
};
