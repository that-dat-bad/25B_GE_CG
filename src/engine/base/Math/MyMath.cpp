#include "MyMath.h"
#include <cmath>

namespace MyMath {

	float LerpShort(float a, float b, float t) {
		float diff = b - a;
		float PI = 3.14159265f;
		float twoPI = PI * 2.0f;

		while (diff > PI) diff -= twoPI;
		while (diff < -PI) diff += twoPI;

		return a + diff * t;
	}

	Matrix4x4 MakeIdentity4x4() {
		return Identity4x4();
	}

	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
		return MakePerspectiveMatrix(fovY, aspectRatio, nearClip, farClip);
	}

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
		return makeOrthographicmMatrix(left, top, right, bottom, nearClip, farClip);
	}

}
