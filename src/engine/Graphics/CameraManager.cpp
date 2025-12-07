#include "CameraManager.h"
#include <cassert>
using namespace TDEngine;

CameraManager* CameraManager::instance_ = nullptr;

CameraManager* CameraManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new CameraManager();
	}
	return instance_;
}

void CameraManager::Initialize() {
	// デフォルトカメラを作っておく（"default" という名前で）
	CreateCamera("default");
	SetActiveCamera("default");
}

void CameraManager::Finalize() {
	cameras_.clear();
	if (instance_ != nullptr) {
		delete instance_;
		instance_ = nullptr;
	}
}

void CameraManager::CreateCamera(const std::string& name) {
	// 既に同じ名前があったら何もしない（あるいは上書き）
	if (cameras_.contains(name)) {
		return;
	}

	// 新しいカメラを作成してMapに登録
	std::unique_ptr<Camera> newCamera = std::make_unique<Camera>();
	cameras_.insert(std::make_pair(name, std::move(newCamera)));
}

void CameraManager::SetActiveCamera(const std::string& name) {
	// 指定した名前のカメラが存在するかチェック
	if (cameras_.contains(name)) {
		activeCamera_ = cameras_[name].get(); // 生ポインタを取得
	}
}

Camera* CameraManager::GetCamera(const std::string& name) {
	// 指定した名前のカメラがあるか検索
	if (cameras_.contains(name)) {
		// unique_ptr から生ポインタを取り出して返す
		return cameras_[name].get();
	}
	// 見つからなければ nullptr
	return nullptr;
}

void CameraManager::Update() {
	if (activeCamera_) {
		activeCamera_->Update();
	}
}