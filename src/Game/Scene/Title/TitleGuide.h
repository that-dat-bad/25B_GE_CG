#pragma once
#include "Object3d.h"
#include <memory>
#include "math/MyMath.h"

class TitleGuide {
private:
	// オブジェクト
	std::unique_ptr<Object3d> object3d_ = nullptr;

	//点滅制御用のパラメータ
	float blinkParameter_ = 0.0f;

	//点滅の周期
	const float kBlinkSpeed_ = 0.1f;

public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();
	void SetPosition(const Vector3& position); 
};