#include "CameraManager.h"
#include <cassert>

std::unique_ptr<CameraManager> CameraManager::instance_ = nullptr;

CameraManager* CameraManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_.reset(new CameraManager());
	}
	return instance_.get();
}

void CameraManager::Initialize() {
	CreateCamera("default");
	SetActiveCamera("default");
}

void CameraManager::Finalize() {
	cameras_.clear();
}

void CameraManager::CreateCamera(const std::string& name) {
	if (cameras_.contains(name)) {
		return;
	}

	std::unique_ptr<Camera> newCamera = std::make_unique<Camera>();
	cameras_.insert(std::make_pair(name, std::move(newCamera)));
}

void CameraManager::SetActiveCamera(const std::string& name) {
	if (cameras_.contains(name)) {
		activeCamera_ = cameras_[name].get(); 
	}
}

void CameraManager::Update() {
	if (activeCamera_) {
		activeCamera_->Update();
	}
}

void CameraManager::DeleteCamera(const std::string& name) {
	if (cameras_.contains(name)) {
		if (activeCamera_ == cameras_[name].get()) {
			activeCamera_ = nullptr;
		}
		cameras_.erase(name);
	}
}

