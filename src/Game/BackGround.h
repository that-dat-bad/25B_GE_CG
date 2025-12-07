#pragma once
#include "Object3d.h"

class BackGround {
public:
	~BackGround();
	// 初期化 (CameraはManager管理なので引数から削除)
	void Initialize(const std::string& modelName, const MyMath::Vector3& position);
	void Update();
	void Draw();

private:
	Object3d* object3d_ = nullptr;
};