#include "CameraManager.h"
#include <cassert>

CameraManager* CameraManager::instance_ = nullptr;

CameraManager* CameraManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new CameraManager();
	}
	return instance_;
}

void CameraManager::Initialize() {
	// "default" という名前のカメラを作っておく
	CreateCamera("default");
	SetActiveCamera("default");
}

void CameraManager::Update() {
	// アクティブなカメラがあれば更新
	if (activeCamera_) {
		activeCamera_->Update();
	}
}

void CameraManager::Finalize() {
	cameras_.clear();
	delete instance_;
	instance_ = nullptr;
}

void CameraManager::CreateCamera(const std::string& name) {
	// 指定された名前のカメラが既にないかチェック
	if (cameras_.find(name) == cameras_.end()) {
		// 新しいカメラを作成してmapに登録
		cameras_[name] = std::make_unique<Camera>();
	}
}

void CameraManager::SetActiveCamera(const std::string& name) {
	// 指定された名前のカメラが存在するか確認
	auto it = cameras_.find(name);
	if (it != cameras_.end()) {
		// アクティブカメラを切り替える
		activeCamera_ = it->second.get();
	} else {
		// 存在しない名前を指定したら止める
		assert(false && "Camera not found.");
	}
}

Camera* CameraManager::GetActiveCamera() const {
	return activeCamera_;
}

Camera* CameraManager::GetCamera(const std::string& name) {
	auto it = cameras_.find(name);
	if (it != cameras_.end()) {
		return it->second.get();
	}
	return nullptr;
}