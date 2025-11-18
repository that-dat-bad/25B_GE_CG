#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include<string>
#include<vector>
#include "../base/Math/MyMath.h"
class Object3dCommon;
class Object3d
{
private:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	struct MaterialData
	{
		std::string textureFilePath;
	};
	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
	};
public:
	void Initialize(Object3dCommon* object3dCommon);
	//.mtlファイルの読み取り
	static MaterialData LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename);
	//.objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
private:
	Object3dCommon* object3dCommon_ = nullptr;
	//objファイルのデータ
	ModelData modelData_;
};

