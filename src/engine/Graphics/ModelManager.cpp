#include "ModelManager.h"
#include "ModelCommon.h"
#include "DirectXCommon.h"
#include <filesystem>
ModelManager* ModelManager::instance_ = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new ModelManager();
	}
	return instance_;
}

void ModelManager::Initialize(DirectXCommon* dxCommon)
{
	modelCommon_ = new ModelCommon();
	modelCommon_->Initialize(dxCommon);
}

void ModelManager::Finalize()
{
	if (instance_ != nullptr) {
		delete instance_;
		instance_ = nullptr;
	}
}

void ModelManager::LoadModel(const std::string& filePath) {
	// 読み込み済みなら早期リターン
	if (models_.contains(filePath)) {
		return;
	}

	if (!std::filesystem::exists(filePath)) {

		// 絶対パスを取得して表示
		std::filesystem::path absPath = std::filesystem::absolute(filePath);
		std::string message = "Model file not found!\n\nPath:\n" + absPath.string();

		// エラーメッセージボックスを表示
		MessageBoxA(nullptr, message.c_str(), "ModelManager Error", MB_OK | MB_ICONERROR);

		// 強制停止
		assert(false);
		return;
	}

	std::filesystem::path pathObj(filePath);
	std::string directoryPath = pathObj.parent_path().string(); // 例: "Resources"
	std::string filename = pathObj.filename().string();         // 例: "player.obj"

	// モデル生成
	std::unique_ptr<Model> model = std::make_unique<Model>();

	// 初期化 (分解したパスを渡す)
	model->Initialize(modelCommon_, directoryPath, filename);

	// 格納
	models_.insert(std::make_pair(filePath, std::move(model)));
}

void ModelManager::LoadModel(const std::string& directoryPath, const std::string& filename)
{

	if (models_.contains(filename))
	{
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon_, directoryPath, filename);
	models_.insert(std::make_pair(filename, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath)
{
	if (models_.contains(filePath))
	{
		//読み込みモデルを戻り値としてreturn
		return models_.at(filePath).get();
	}

	//ファイル名一致なし
	return nullptr;
}
