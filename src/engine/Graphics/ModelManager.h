#pragma once
#include<map>
#include <string>
#include <memory>
#include"Model.h"

namespace TDEngine {
	class ModelCommon;
	class DirectXCommon;
	class ModelManager
	{

	public:
		static ModelManager* GetInstance();
		void Initialize(DirectXCommon* dxCommon);
		void Finalize();
		~ModelManager() = default;
		/// <summary>
		/// モデルファイルの読み込み
		/// </summary>
		/// <param name="filePath"></param>
		void LoadModel(const std::string& filePath);
		void LoadModel(const std::string& directoryPath, const std::string& filename);

		/// <summary>
		/// モデルの検索
		/// </summary>
		/// <param name="filePath"></param>
		/// <returns></returns>
		Model* FindModel(const std::string& filePath);
	private:
		static ModelManager* instance_;
		ModelManager() = default;

		ModelManager(const ModelManager&) = delete;
		ModelManager& operator=(const ModelManager&) = delete;
		ModelCommon* modelCommon_ = nullptr;

		//モデルデータ
		std::map<std::string, std::unique_ptr<Model>> models_;
	};

}