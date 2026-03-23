#include "Matrix4x4.h"
#pragma once

namespace MyMath {
	struct Quaternion {
		float x, y, z, w;
	};
	Matrix4x4 MakeRotateMatrix(const Quaternion& q);


	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate);

	// クォータニオンの球面線形補間 (回転を滑らかに繋ぐ)
	Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);
}
