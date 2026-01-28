#include "ModelManager.h"
#include "ModelCommon.h"
#include "DirectXCommon.h"
#include <filesystem>
#include <vector>
#include <string>

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
	
	std::unique_ptr<Model> model = std::make_unique<Model>();
	
	std::filesystem::path p(filePath);
	std::string stem = p.stem().string();
	std::string ext = p.extension().string();

	// 探索ベースとなるパス（ルート、assets、Resourcesなど）
	std::vector<std::string> basePaths = {
		"",
		"assets/",
		"Resources/",
		"assets/models/"
	};

	std::string foundPath = "";
	bool found = false;

	for (const auto& base : basePaths) {
		std::string tryPath = base + filePath;
		
		// 1. 指定されたパスそのまま、もしくは .obj つきで存在するか
		if (std::filesystem::exists(tryPath) && !std::filesystem::is_directory(tryPath)) {
			foundPath = tryPath;
			found = true;
			break;
		}
		if (std::filesystem::exists(tryPath + ".obj")) {
			foundPath = tryPath + ".obj";
			found = true;
			break;
		}

		// 2. 拡張子がない場合、同名フォルダ内の同名 .obj を探す (例: title/title.obj)
		if (ext.empty()) {
			std::string folderObjPath = tryPath + "/" + stem + ".obj";
			if (std::filesystem::exists(folderObjPath)) {
				foundPath = folderObjPath;
				found = true;
				break;
			}
		}
	}

	// 見つからなかった場合のデフォルト
	if (!found) {
		foundPath = "assets/" + filePath;
	}

	model->Initialize(modelCommon_.get(), "", foundPath);

	models_.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath)
{
	if (models_.contains(filePath))
	{
		return models_.at(filePath).get();
	}

	return nullptr;
}

