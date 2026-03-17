#pragma once
#include <cmath>
#include "Vector3.h"
#include "Matrix4x4.h"

namespace MyMath {

	struct Quaternion {
		float x, y, z, w;
	};

	// 単位クォータニオン
	inline Quaternion IdentityQuaternion() {
		return { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	// クォータニオンの積
	inline Quaternion MultiplyQ(const Quaternion& q1, const Quaternion& q2) {
		return {
			q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
			q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
			q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
			q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
		};
	}

	// クォータニオンの共役
	inline Quaternion ConjugateQ(const Quaternion& q) {
		return { -q.x, -q.y, -q.z, q.w };
	}

	// クォータニオンのノルム
	inline float NormQ(const Quaternion& q) {
		return std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	}

	// クォータニオンの正規化
	inline Quaternion NormalizeQ(const Quaternion& q) {
		float n = NormQ(q);
		if (n < 1e-6f) return IdentityQuaternion();
		return { q.x / n, q.y / n, q.z / n, q.w / n };
	}

	// 球面線形補間 (Slerp)
	inline Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {
		float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;

		Quaternion q1n = q1;
		// 内積が負の場合、反転
		if (dot < 0.0f) {
			q1n = { -q1.x, -q1.y, -q1.z, -q1.w };
			dot = -dot;
		}

		// ほぼ同じ向きの場合はLerp
		if (dot > 0.9995f) {
			Quaternion result = {
				q0.x + t * (q1n.x - q0.x),
				q0.y + t * (q1n.y - q0.y),
				q0.z + t * (q1n.z - q0.z),
				q0.w + t * (q1n.w - q0.w)
			};
			return NormalizeQ(result);
		}

		float theta_0 = std::acos(dot);
		float theta = theta_0 * t;
		float sin_theta = std::sin(theta);
		float sin_theta_0 = std::sin(theta_0);

		float s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
		float s1 = sin_theta / sin_theta_0;

		return {
			s0 * q0.x + s1 * q1n.x,
			s0 * q0.y + s1 * q1n.y,
			s0 * q0.z + s1 * q1n.z,
			s0 * q0.w + s1 * q1n.w
		};
	}

	// クォータニオンから回転行列への変換
	inline Matrix4x4 MakeRotateMatrixFromQuaternion(const Quaternion& q) {
		Matrix4x4 result;
		float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
		float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
		float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

		result.m[0][0] = 1.0f - 2.0f * (yy + zz);
		result.m[0][1] = 2.0f * (xy + wz);
		result.m[0][2] = 2.0f * (xz - wy);
		result.m[0][3] = 0.0f;

		result.m[1][0] = 2.0f * (xy - wz);
		result.m[1][1] = 1.0f - 2.0f * (xx + zz);
		result.m[1][2] = 2.0f * (yz + wx);
		result.m[1][3] = 0.0f;

		result.m[2][0] = 2.0f * (xz + wy);
		result.m[2][1] = 2.0f * (yz - wx);
		result.m[2][2] = 1.0f - 2.0f * (xx + yy);
		result.m[2][3] = 0.0f;

		result.m[3][0] = 0.0f;
		result.m[3][1] = 0.0f;
		result.m[3][2] = 0.0f;
		result.m[3][3] = 1.0f;

		return result;
	}

	// Vec3の線形補間
	inline Vector3 LerpV3(const Vector3& v0, const Vector3& v1, float t) {
		return {
			v0.x + t * (v1.x - v0.x),
			v0.y + t * (v1.y - v0.y),
			v0.z + t * (v1.z - v0.z)
		};
	}

}
