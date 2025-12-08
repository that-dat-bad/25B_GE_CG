#pragma once
#include "Sprite.h"

class Fade {
public:
	enum class Status { kNone, kFadeIn, kFadeOut };
	Status status_ = Status::kNone;

	~Fade();
	void Initialize();
	void Update();
	void Draw();
	void Start(Status status, float duration);
	void Stop();
	bool IsFinished() const;

private:
	void UpdateFadeIn();
	void UpdateFadeOut();

	Sprite* sprite_ = nullptr;
	bool isActive_ = false;
	float timer_ = 0.0f;
	float duration_ = 0.0f;

	float startScale_ = 0.1f;
	float endScale_ = 1800.0f;
};