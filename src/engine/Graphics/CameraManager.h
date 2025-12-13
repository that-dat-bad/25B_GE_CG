#pragma once
#include "Camera.h"
#include <map>
#include <string>
#include <memory>

class CameraManager {
public: 
	static CameraManager* GetInstance();
	~CameraManager() = default;
	void Initialize();
	void Finalize();

	// カメラの生成 (newしてmapに登録)
	void CreateCamera(const std::string& name);

	// アクティブカメラの切り替え
	void SetActiveCamera(const std::string& name);

	// アクティブカメラの取得 (描画やオブジェクト側で使う)
	Camera* GetActiveCamera() const { return activeCamera_; }

	// 更新 (アクティブなカメラだけ更新する)
	void Update();

	//指定されたカメラの削除
	void DeleteCamera(const std::string& name);

private:
	static CameraManager* instance_;
	CameraManager() = default;

	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

	// カメラのマップ (名前, 実体)
	std::map<std::string, std::unique_ptr<Camera>> cameras_;

	// 現在有効なカメラへのポインタ (所有権は持たない)
	Camera* activeCamera_ = nullptr;
};