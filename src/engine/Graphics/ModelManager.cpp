#include "ModelManager.h"
#include "ModelCommon.h"
#include "DirectXCommon.h"

std::unique_ptr<ModelManager> ModelManager::instance_ = nullptr;

ModelManager::ModelManager() = default;
ModelManager::~ModelManager() = default;

ModelManager* ModelManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_.reset(new ModelManager());
	}
	return instance_.get();
}

void ModelManager::Initialize(DirectXCommon* dxCommon)
{
	modelCommon_ = std::make_unique<ModelCommon>();
	modelCommon_->Initialize(dxCommon);
}

void ModelManager::Finalize()
{
	instance_.reset();
}

void ModelManager::LoadModel(const std::string& filePath)
{
	if (models_.contains(filePath))
	{
		return;
	}
	
	//モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	//model->Initialize(modelCommon_, "assets", filePath);
	std::string fullPath = "assets/" + filePath;
	model->Initialize(modelCommon_.get(), "", fullPath);

	//モデルをmapコンテナに格納する
	models_.insert(std::make_pair(filePath, std::move(model)));
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
