#pragma once
#include "../base/Math/MyMath.h"
using namespace MyMath;
#include <dinput.h>    


class DebugCamera {
public:
	/// <summary>
	/// </summary>
	DebugCamera();

	/// <summary>
	/// </summary>
	void Update(const BYTE* key, const DIMOUSESTATE2& mouseState);

	/// <summary>
	/// </summary>
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }

private:
	Matrix4x4 viewMatrix_ = MakeIdentity4x4();

	Vector3 pivot_ = { 0.0f, 0.0f, 0.0f };

	float distance_ = 10.0f;

	Vector2 rotation_ = { -0.3f, 1.57f }; // (pitch, yaw)

	Vector2 prevMousePos_ = { 0.0f, 0.0f };
};
