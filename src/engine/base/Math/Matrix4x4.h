#pragma once
#include "Vector3.h"

namespace MyMath {
	struct Matrix4x4 {
		float m[4][4];
	};

	// 和
	Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

	// 差
	Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

	// 積
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	// 逆行列
	Matrix4x4 Inverse(const Matrix4x4& m);

	// 転置行列
	Matrix4x4 Transpose(const Matrix4x4& m);

	// 単位行列
	Matrix4x4 Identity4x4();

	// 平行移動行列
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	//拡大縮小行列
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);


	Matrix4x4 MakeRotateXMatrix(float angle);

	Matrix4x4 MakeRotateYMatrix(float angle);

	Matrix4x4 MakeRotateZMatrix(float angle);

	//アフィン変換行列
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);


	//座標変換
	Vector3 TransformV3(const Vector3& vector, const Matrix4x4& matrix);


	//透視投影行列
	Matrix4x4 MakePerspectiveMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	//正射影行列
	Matrix4x4 makeOrthographicmMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	//ビューポート行列
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

	// 行列の掛け算演算子オーバーロード
	inline Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs) {
		Matrix4x4 result;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.m[i][j] = 0.0f;
				for (int k = 0; k < 4; ++k) {
					result.m[i][j] += lhs.m[i][k] * rhs.m[k][j];
				}
			}
		}
		return result;
	}
}
