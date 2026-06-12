#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../../engine/base/Math/Vector3.h"

struct ColliderData {
    std::string type;
    MyMath::Vector3 center = {0,0,0};
    MyMath::Vector3 size = {0,0,0};
};

struct ObjectData {
    std::string type;
    std::string name;
    MyMath::Vector3 translation = {0,0,0};
    MyMath::Vector3 rotation = {0,0,0};
    MyMath::Vector3 scaling = {1,1,1};
    std::string fileName;
    ColliderData collider;
    bool hasCollider = false;
    std::string spawnType;
    std::vector<ObjectData> children;
};

struct LevelData {
    std::string name;
    std::vector<ObjectData> objects;
};

class LevelLoader {
public:
    static std::unique_ptr<LevelData> LoadJSON(const std::string& filePath);
};
