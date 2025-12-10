#pragma once
#include "Sprite.h"
#include <vector>
#include <array>
#include <string>

class TimeLimit {
public:
	~TimeLimit();
	void Initialize();
	void Update();
	void Draw();

	bool IsStartCountDown() const { return phase_ == Phase::kStartCountDown; }
	bool IsTImeUp() const { return phase_ == Phase::kTimeUp; }

	enum class Phase { kStartCountDown, kActive, kLast5Second, kTimeUp };
	Phase phase_ = Phase::kStartCountDown;

	float timer_ = 0.0f;
	static inline const float kTimeLimit = 60.0f; // 短め？（元コード通り）
	float countDown_ = 3.0f;

private:
	// テクスチャファイルパスを保持するか、SpriteごとにSetTextureするか
	// ここではSpriteインスタンスを管理し、TextureManagerは内部で利用
	std::vector<std::string> digitTexturePaths_;
	std::string colonTexturePath_;

	static const int kMaxGlyphs = 6;
	std::array<Sprite*, kMaxGlyphs> glyphs_{};
	Sprite* centerDigit_ = nullptr;

private:

	void LayoutGlyphs(Vector2 size);
	void UpdateGlyphTexturesFromTime();
};