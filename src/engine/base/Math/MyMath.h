#pragma once
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include"Matrix4x4.h"
#include"Transform.h"

namespace MyMath {
	float LerpShort(float a, float b, float t);

	// Wrapper functions to match user code with engine implementation
	Matrix4x4 MakeIdentity4x4();
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	// 座標変換 (ベクトル * 行列) w=1として計算
	inline Vector3 TransformCoord(const Vector3& v, const Matrix4x4& m) {
		float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
		Vector3 result;
		// 0除算対策
		if (w == 0.0f) w = 1.0f;
		
		result.x = (v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0]) / w;
		result.y = (v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1]) / w;
		result.z = (v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2]) / w;
		return result;
	}

	// 3Dワールド座標を2Dスクリーン座標に変換
	inline Vector2 WorldToScreen(const Vector3& worldPos, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, float screenWidth, float screenHeight) {
		// ビュープロジェクション行列
		Matrix4x4 viewProj = Multiply(viewMatrix, projMatrix);
		
		// ワールド座標をクリップ座標に変換
		float x = worldPos.x * viewProj.m[0][0] + worldPos.y * viewProj.m[1][0] + worldPos.z * viewProj.m[2][0] + viewProj.m[3][0];
		float y = worldPos.x * viewProj.m[0][1] + worldPos.y * viewProj.m[1][1] + worldPos.z * viewProj.m[2][1] + viewProj.m[3][1];
		float w = worldPos.x * viewProj.m[0][3] + worldPos.y * viewProj.m[1][3] + worldPos.z * viewProj.m[2][3] + viewProj.m[3][3];
		
		// wで割って正規化デバイス座標に変換
		if (w != 0.0f) {
			x /= w;
			y /= w;
		}
		
		// NDC座標(-1～1) をスクリーン座標に変換
		Vector2 screenPos;
		screenPos.x = (x + 1.0f) * 0.5f * screenWidth;
		screenPos.y = (1.0f - y) * 0.5f * screenHeight; // Yは反転
		
		return screenPos;
	}
}
