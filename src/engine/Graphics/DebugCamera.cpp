#include "DebugCamera.h"
#include "../base/Math/MyMath.h"
using namespace MyMath;
#include<cmath>


Matrix4x4 MakeLookAtMatrix(const Vector3& eye, const Vector3& target, const Vector3& up) {
    Vector3 zaxis = Normalize(Substract(target, eye));
    Vector3 xaxis = Normalize(Cross(up, zaxis));
    Vector3 yaxis = Cross(zaxis, xaxis);
    Matrix4x4 viewMatrix = {
        xaxis.x, yaxis.x, zaxis.x, 0,
        xaxis.y, yaxis.y, zaxis.y, 0,
        xaxis.z, yaxis.z, zaxis.z, 0,
        -Dot(xaxis, eye), -Dot(yaxis, eye), -Dot(zaxis, eye), 1.0f
    };
    return viewMatrix;
}
DebugCamera::DebugCamera() {
    Update(nullptr, {});
}

void DebugCamera::Update(const BYTE* key, const DIMOUSESTATE2& mouseState) {

    if (mouseState.rgbButtons[1] & 0x80) {
        float dx = static_cast<float>(mouseState.lX) * 0.005f;
        float dy = static_cast<float>(mouseState.lY) * 0.005f;
        rotation_.y += dx; 
        rotation_.x += dy; 

        float pitchLimit = 3.141592f * 0.49f; 
        if (rotation_.x > pitchLimit) rotation_.x = pitchLimit;
        if (rotation_.x < -pitchLimit) rotation_.x = -pitchLimit;
    }

    float wheel = static_cast<float>(mouseState.lZ) * 0.01f;
    distance_ -= wheel;
    if (distance_ < 1.0f) {
        distance_ = 1.0f;
    }

    if (mouseState.rgbButtons[2] & 0x80) {
        float dx = static_cast<float>(mouseState.lX) * -0.01f;
        float dy = static_cast<float>(mouseState.lY) * 0.01f;

        Matrix4x4 cameraWorldMatrix = Inverse(viewMatrix_);
        Vector3 cameraRight = { cameraWorldMatrix.m[0][0], cameraWorldMatrix.m[0][1], cameraWorldMatrix.m[0][2] };
        Vector3 cameraUp = { cameraWorldMatrix.m[1][0], cameraWorldMatrix.m[1][1], cameraWorldMatrix.m[1][2] };

        Vector3 move = Add(Multiply(dx, cameraRight), Multiply(dy, cameraUp));
        pivot_ = Add(pivot_, move);
    }


    Vector3 offset;
    offset.x = distance_ * std::cos(rotation_.x) * std::sin(rotation_.y);
    offset.y = distance_ * std::sin(rotation_.x);
    offset.z = distance_ * std::cos(rotation_.x) * std::cos(rotation_.y);

    Vector3 eyePosition = Add(pivot_, offset);

    viewMatrix_ = MakeLookAtMatrix(eyePosition, pivot_, { 0.0f, 1.0f, 0.0f });
}
