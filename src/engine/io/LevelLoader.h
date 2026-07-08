#pragma once
#include <string>
#include <vector>
#include "../base/Math/MyMath.h"
#include "json.hpp"

struct LevelData {
    struct ColliderData {
        std::string type;
        MyMath::Vector3 center;
        MyMath::Vector3 size;
    };

    struct ObjectData {
        std::string name;
        std::string type;
        std::string fileName;
        MyMath::Vector3 translation;
        MyMath::Vector3 rotation;
        MyMath::Vector3 scaling;
        bool disabled = false;

        bool hasCollider = false;
        ColliderData collider;

        std::vector<ObjectData> children;
    };

    std::vector<ObjectData> objects;
};

class LevelLoader {
public:
    static const std::string kDefaultBaseDirectory;
    static const std::string kExtension;

    /// @brief レベルデータをJSONファイルから読み込む
    /// @param fileName ファイル名 (拡張子・ベースディレクトリを除く)
    /// @return 読み込んだレベルデータ (呼び出し元がdeleteすること)
    static LevelData* Load(const std::string& fileName);

private:
    /// @brief オブジェクトノードを再帰的にパースする
    /// @param object JSONオブジェクト
    /// @param levelData レベルデータ格納用ポインタ
    /// @param parentData 親オブジェクトデータ（階層構造の構築用）
    static void ParseObject(const nlohmann::json& object, LevelData* levelData, LevelData::ObjectData* parentData = nullptr);
};
