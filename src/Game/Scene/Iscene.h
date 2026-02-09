#pragma once
#include "Input.h"
#include <optional>

enum class SceneID {
	kTitle,
	kStage,
	kClear
};

class IScene {
public:

	enum ScenePhase {
		kFadeIn,
		kMain,
		kFadeOut,
	};

protected:

	ScenePhase phase_ = ScenePhase::kFadeIn;
	int fadeTimer_ = 30;
	const int kFadeDuration_ = 30;

public:

	virtual void Initialize() = 0;
	virtual std::optional<SceneID> Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;
	virtual ~IScene() = default;

};

