#pragma once
#include<map>
#include <string>
#include <memory>
#include"Model.h"
class ModelCommon;
class DirectXCommon;
class ModelManager
{

public:
	static ModelManager* GetInstance();
	void Initialize(DirectXCommon* dxCommon);
	void Finalize();

	/// <summary>
	/// </summary>
	/// <param name="filePath"></param>
	void LoadModel(const std::string& filePath);

	/// <summary>
	/// </summary>
	/// <param name="filePath"></param>
	/// <returns></returns>
	Model* FindModel(const std::string& filePath);
	~ModelManager();
private:
	static std::unique_ptr<ModelManager> instance_;
	ModelManager();
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
	std::unique_ptr<ModelCommon> modelCommon_ = nullptr;

	std::map<std::string, std::unique_ptr<Model>> models_;
};


