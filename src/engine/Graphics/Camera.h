#pragma once
#include "../base/Math/MyMath.h"
using namespace MyMath;

class Camera
{
public:
	Camera();
	void Update();

	//アクセッサ
	//--セッター--//
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetFovY(const float fovY) { fovY_ = fovY; }
	void SetAspectRatio(const float aspectRatio) { aspectRatio_ = aspectRatio; }
	void SetNearClip(const float nearClip) { nearClip_ = nearClip; }
	void SetFarClip(const float farClip) { farClip_ = farClip; }

	//--ゲッター--//
	const Matrix4x4& GetWorldMatrix()const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const{ return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const{ return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const{ return viewProjectionMatrix_; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }
private:
	Transform transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	//水平方向視野角
	float fovY_;
	//アスペクト比
	float aspectRatio_;
	//ニアクリップ距離
	float nearClip_;
	//ファークリップ
	float farClip_;
};

