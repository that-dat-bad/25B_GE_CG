#include "LevelLoader.h"
#include <fstream>
#include <cassert>

const std::string LevelLoader::kDefaultBaseDirectory = "Resources/";
const std::string LevelLoader::kExtension = ".json";

LevelData* LevelLoader::Load(const std::string& fileName) {
    // 連結してフルパスを得る
    const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

    // ファイルストリーム
    std::ifstream file;

    // ファイルを開く
    file.open(fullpath);
    
    // ファイルオープン失敗をチェック
    if (file.fail()) {
        assert(0 && "Failed to open level JSON file.");
        return nullptr;
    }

    // JSON文字列から解凍したデータ
    nlohmann::json deserialized;

    // 解凍
    file >> deserialized;

    // 正しいレベルデータファイルかチェック
    assert(deserialized.is_object());
    assert(deserialized.contains("name"));
    assert(deserialized["name"].is_string());

    // "name"を文字列として取得
    std::string name = deserialized["name"].get<std::string>();
    
    // 正しいレベルデータファイルかチェック
    assert(name.compare("scene") == 0);

    // レベルデータ格納用インスタンスを生成
    LevelData* levelData = new LevelData();

    // "objects"の全オブジェクトを走査
    if (deserialized.contains("objects") && deserialized["objects"].is_array()) {
        for (const nlohmann::json& object : deserialized["objects"]) {
            ParseObject(object, levelData);
        }
    }

    return levelData;
}

void LevelLoader::ParseObject(const nlohmann::json& object, LevelData* levelData, LevelData::ObjectData* parentData) {
    assert(object.contains("type"));
    
    LevelData::ObjectData objectData;

    // オブジェクト情報の取得
    if (object.contains("name")) {
        objectData.name = object["name"].get<std::string>();
    }
    if (object.contains("type")) {
        objectData.type = object["type"].get<std::string>();
    }
    if (object.contains("disabled")) {
        // bool型として取得。無い場合はデフォルトでfalse
        objectData.disabled = object["disabled"].get<bool>();
    }
    if (object.contains("file_name")) {
        objectData.fileName = object["file_name"].get<std::string>();
    }

    // トランスフォーム情報の読み込みと座標系変換 (Blender -> DirectX)
    if (object.contains("transform")) {
        const auto& transform = object["transform"];

        // 平行移動 (location または translation)
        std::string locKey = transform.contains("translation") ? "translation" : "location";
        if (transform.contains(locKey)) {
            objectData.translation.x = (float)transform[locKey][0];
            objectData.translation.y = (float)transform[locKey][2]; // Blender Z -> Game Y
            objectData.translation.z = (float)transform[locKey][1]; // Blender Y -> Game Z
        } else {
            objectData.translation = { 0.0f, 0.0f, 0.0f };
        }

        // 回転角 (rotation) - 符号反転
        if (transform.contains("rotation")) {
            objectData.rotation.x = -(float)transform["rotation"][0];
            objectData.rotation.y = -(float)transform["rotation"][2]; // Blender Z -> Game Y
            objectData.rotation.z = -(float)transform["rotation"][1]; // Blender Y -> Game Z
        } else {
            objectData.rotation = { 0.0f, 0.0f, 0.0f };
        }

        // スケール (scale または scaling)
        std::string scaleKey = transform.contains("scaling") ? "scaling" : "scale";
        if (transform.contains(scaleKey)) {
            objectData.scaling.x = (float)transform[scaleKey][0];
            objectData.scaling.y = (float)transform[scaleKey][2]; // Blender Z -> Game Y
            objectData.scaling.z = (float)transform[scaleKey][1]; // Blender Y -> Game Z
        } else {
            objectData.scaling = { 1.0f, 1.0f, 1.0f };
        }
    } else {
        objectData.translation = { 0.0f, 0.0f, 0.0f };
        objectData.rotation = { 0.0f, 0.0f, 0.0f };
        objectData.scaling = { 1.0f, 1.0f, 1.0f };
    }

    // コライダー情報の読み込み
    if (object.contains("collider")) {
        const auto& col = object["collider"];
        objectData.hasCollider = true;
        if (col.contains("type")) {
            objectData.collider.type = col["type"].get<std::string>();
        }
        if (col.contains("center")) {
            objectData.collider.center.x = (float)col["center"][0];
            objectData.collider.center.y = (float)col["center"][2];
            objectData.collider.center.z = (float)col["center"][1];
        } else {
            objectData.collider.center = { 0.0f, 0.0f, 0.0f };
        }
        if (col.contains("size")) {
            objectData.collider.size.x = (float)col["size"][0];
            objectData.collider.size.y = (float)col["size"][2];
            objectData.collider.size.z = (float)col["size"][1];
        } else {
            objectData.collider.size = { 1.0f, 1.0f, 1.0f };
        }
    }

    // 子オブジェクトの再帰的処理
    if (object.contains("children") && object["children"].is_array()) {
        for (const auto& childJson : object["children"]) {
            // 子要素をパースする際、このオブジェクトの子供リストに追加するため、コピーを保持する
            // 参照の無効化を避けるため、一時的な vector にためてから objectData.children に追加する
            LevelData::ObjectData childData;
            ParseObject(childJson, levelData, &childData);
            objectData.children.push_back(childData);
        }
    }

    // フラットなリストと親オブジェクトの子供リストに追加
    levelData->objects.push_back(objectData);

    if (parentData != nullptr) {
        *parentData = objectData;
    }
}
