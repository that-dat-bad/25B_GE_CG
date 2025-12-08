#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"

namespace MyMath {
    float EaseIn(float t, float start, float end);
    float EaseOut(float t, float start, float end);
    float Lerp(float start, float end, float t);
    Vector3 Lerp(const Vector3& start, const Vector3& end, float t);
    Vector3 EaseIn(float t, const Vector3& start, const Vector3& end);
    Vector3 EaseOut(float t, const Vector3& start, const Vector3& end);
    float EaseInOutSine(float t);
}