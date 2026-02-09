#include "Camera.h"
#include"../base/WinApp.h"
#include <cstdlib>  // for rand()

Camera::Camera()

	: transform_({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
	, fovY_(0.45f)
	, aspectRatio_(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip_(0.1f)
	, farClip_(100.0f)
	, worldMatrix_(MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate))
	, viewMatrix_(Inverse(worldMatrix_))
	, projectionMatrix_(MakePerspectiveMatrix(fovY_, aspectRatio_, nearClip_, farClip_))
	, viewProjectionMatrix_(Multiply(viewMatrix_, projectionMatrix_))
{
}

void Camera::Shake(float intensity, int durationFrames) {
	shakeIntensity_ = intensity;
	shakeDuration_ = durationFrames;
	shakeTimer_ = durationFrames;
}

void Camera::Update() {
	// シェイクオフセットを計算
	Vector3 shakeOffset = { 0.0f, 0.0f, 0.0f };
	if (shakeTimer_ > 0) {
		// 減衰係数（時間が経つほど弱くなる）
		float decay = static_cast<float>(shakeTimer_) / static_cast<float>(shakeDuration_);
		float currentIntensity = shakeIntensity_ * decay;

		// ランダムなオフセット (-1.0 〜 1.0)
		float randX = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
		float randY = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;

		shakeOffset.x = randX * currentIntensity;
		shakeOffset.y = randY * currentIntensity;

		shakeTimer_--;
	}

	// シェイクを適用した位置でワールド行列を計算
	Vector3 shakenTranslate = {
		transform_.translate.x + shakeOffset.x,
		transform_.translate.y + shakeOffset.y,
		transform_.translate.z + shakeOffset.z
	};

	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, shakenTranslate);
	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}
