#pragma once

/// <summary>
/// </summary>

namespace MyMath {
	struct Vector3 {
		float x;
		float y;
		float z;
	};
	Vector3 Add(const Vector3& v1, const Vector3& v2);

	Vector3 Substract(const Vector3& v1, const Vector3& v2);

	Vector3 Multiply(float scaler, const Vector3& v);

	float Dot(const Vector3& v1, const Vector3& v2);

	float Length(const Vector3& v);

	Vector3 Normalize(const Vector3& v);

	Vector3 Cross(const Vector3& v1, const Vector3& v2);
}
