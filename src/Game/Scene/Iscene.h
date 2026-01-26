#pragma once
#include "Input.h"
#include <optional>

// シーン識別子
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
	// シーン遷移リクエストがあればSceneIDを返す
	virtual std::optional<SceneID> Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;
	virtual ~IScene() = default;

};
