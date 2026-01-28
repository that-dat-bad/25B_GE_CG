#pragma once
#include "Vector3.h"
/// <summary>
/// </summary>
namespace MyMath {
	struct Matrix4x4 final {
		float m[4][4];
	};

	Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

	Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	Matrix4x4 Inverse(const Matrix4x4& m);

	Matrix4x4 Transpose(const Matrix4x4& m);

	Matrix4x4 Identity4x4();

	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	Matrix4x4 MakeScaleMatrix(const Vector3& scale);


	Matrix4x4 MakeRotateXMatrix(float angle);

	Matrix4x4 MakeRotateYMatrix(float angle);

	Matrix4x4 MakeRotateZMatrix(float angle);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);


	Vector3 TransformV3(const Vector3& vector, const Matrix4x4& matrix);


	Matrix4x4 MakePerspectiveMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	Matrix4x4 makeOrthographicmMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);
}
