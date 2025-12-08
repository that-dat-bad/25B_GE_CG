#include "MyMath.h"
#include <cmath>
namespace MyMath {
    float EaseIn(float t, float start, float end) {
        float easeT = t * t;
        return (1.0f - easeT) * start + easeT * end;
    }

    float EaseOut(float t, float start, float end) {
        float easeT = 1.0f - (1.0f - t) * (1.0f - t);
        return (1.0f - easeT) * start + easeT * end;
    }

    float Lerp(float start, float end, float t) {
        return start + (end - start) * t;
    }

    Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
        Vector3 ret;
        ret.x = start.x + (end.x - start.x) * t;
        ret.y = start.y + (end.y - start.y) * t;
        ret.z = start.z + (end.z - start.z) * t;
        return ret;
    }

    Vector3 EaseIn(float t, const Vector3& start, const Vector3& end) {
        float easeT = t * t;
        return Lerp(start, end, easeT);
    }

    Vector3 EaseOut(float t, const Vector3& start, const Vector3& end) {
        float easeT = 1.0f - (1.0f - t) * (1.0f - t);
        return Lerp(start, end, easeT);
    }
    float EaseInOutSine(float t) {
        return -(std::cos(3.14159265f * t) - 1.0f) * 0.5f;
    }
}