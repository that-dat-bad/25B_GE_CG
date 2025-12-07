#include "ModelManager.h"
#include "ModelCommon.h"
#include "DirectXCommon.h"
#include <filesystem>

using namespace TDEngine;

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

void ModelManager::LoadModel(const std::string& filePath)
{
	// 読み込み済みなら早期リターン
	if (models_.contains(filePath))
	{
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();

	// デフォルトは "assets" 直下と仮定
	std::string directoryPath = "assets";

	// 探したいファイルのパスを作成 ("assets/axis.obj" など)
	std::string searchPath = directoryPath + "/" + filePath;

	// 1. 直下に存在するかチェック
	if (!std::filesystem::exists(searchPath)) {

		// 2. なければ assets 以下の全フォルダを再帰的に探索する
		bool found = false;
		// recursive_directory_iterator はサブフォルダも全部潜ってくれます
		for (const auto& entry : std::filesystem::recursive_directory_iterator("assets")) {

			// フォルダではなく「ファイル」であるか確認
			if (entry.is_regular_file()) {

				// ファイル名が一致するか確認 (例: "axis.obj" == "axis.obj")
				if (entry.path().filename().string() == filePath) {

					// 見つかった！
					// そのファイルがある「ディレクトリパス」を取得
					// entry.path().parent_path() は "assets/someFolder" のようなパスを返します
					directoryPath = entry.path().parent_path().string();
					found = true;
					break; // 見つかったらループ終了
				}
			}
		}

		// それでも見つからなかった場合
		if (!found) {

		}
	}

	// 発見した（またはデフォルトの）ディレクトリパスを渡して初期化
	model->Initialize(modelCommon_, directoryPath, filePath);

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
