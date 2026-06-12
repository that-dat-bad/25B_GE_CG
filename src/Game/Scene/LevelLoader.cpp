#include "LevelLoader.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include "../../engine/io/json.hpp" // Path to downloaded json.hpp

using json = nlohmann::json;

void ParseObjectData(const json& jsonObject, ObjectData& objectData) {
    if (jsonObject.contains("type")) objectData.type = jsonObject["type"].get<std::string>();
    if (jsonObject.contains("name")) objectData.name = jsonObject["name"].get<std::string>();

    if (jsonObject.contains("transform")) {
        auto& transform = jsonObject["transform"];
        if (transform.contains("translation")) {
            objectData.translation.x = (float)transform["translation"][0];
            objectData.translation.y = (float)transform["translation"][1];
            objectData.translation.z = (float)transform["translation"][2];
        }
        if (transform.contains("rotation")) {
            // Blender Z-up to DirectX Y-up coordinate conversion
            objectData.rotation.x = -(float)transform["rotation"][0];
            objectData.rotation.y = -(float)transform["rotation"][2];
            objectData.rotation.z = -(float)transform["rotation"][1];
        }
        if (transform.contains("scaling")) {
            objectData.scaling.x = (float)transform["scaling"][0];
            objectData.scaling.y = (float)transform["scaling"][1];
            objectData.scaling.z = (float)transform["scaling"][2];
        }
    }

    if (jsonObject.contains("file_name")) {
        objectData.fileName = jsonObject["file_name"].get<std::string>();
    }

    if (jsonObject.contains("spawn_type")) {
        objectData.spawnType = jsonObject["spawn_type"].get<std::string>();
    }

    if (jsonObject.contains("collider")) {
        objectData.hasCollider = true;
        auto& coll = jsonObject["collider"];
        if (coll.contains("type")) objectData.collider.type = coll["type"].get<std::string>();
        if (coll.contains("center")) {
            objectData.collider.center.x = (float)coll["center"][0];
            objectData.collider.center.y = (float)coll["center"][1];
            objectData.collider.center.z = (float)coll["center"][2];
        }
        if (coll.contains("size")) {
            objectData.collider.size.x = (float)coll["size"][0];
            objectData.collider.size.y = (float)coll["size"][1];
            objectData.collider.size.z = (float)coll["size"][2];
        }
    }

    if (jsonObject.contains("children")) {
        for (const auto& childJson : jsonObject["children"]) {
            ObjectData childData;
            ParseObjectData(childJson, childData);
            objectData.children.push_back(childData);
        }
    }
}

std::unique_ptr<LevelData> LevelLoader::LoadJSON(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open level data file: " << filePath << std::endl;
        return nullptr;
    }

    json deserialized;
    file >> deserialized;

    assert(deserialized.is_object());
    assert(deserialized.contains("name"));
    assert(deserialized["name"].is_string());
    
    std::string name = deserialized["name"].get<std::string>();
    assert(name == "scene");

    std::unique_ptr<LevelData> levelData = std::make_unique<LevelData>();
    levelData->name = name;

    if (deserialized.contains("objects")) {
        for (const auto& object : deserialized["objects"]) {
            ObjectData objectData;
            ParseObjectData(object, objectData);
            levelData->objects.push_back(objectData);
        }
    }

    return levelData;
}
