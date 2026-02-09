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

	void CreateCamera(const std::string& name);

	void SetActiveCamera(const std::string& name);

	Camera* GetActiveCamera() const { return activeCamera_; }

	void Update();

	void DeleteCamera(const std::string& name);

private:
	static std::unique_ptr<CameraManager> instance_;
	CameraManager() = default;

	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

	std::map<std::string, std::unique_ptr<Camera>> cameras_;

	Camera* activeCamera_ = nullptr;
};
