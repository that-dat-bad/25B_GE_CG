#include "Camera.h"
#include"../base/WinApp.h"
Camera::Camera()
{
	: transform_({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, {0.0f,0.0f,0.0f})
		,fovY_(0.45f)
		,aspectRatio_(float(WinApp::clientWidth)/float(WinApp::kClientHeight))

}
void Camera::Update() {
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}