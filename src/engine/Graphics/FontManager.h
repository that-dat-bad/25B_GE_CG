#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../../../external/stb/stb_truetype.h"
#include "../base/Math/MyMath.h"

using namespace MyMath;

struct CharacterInfo {
    stbtt_bakedchar cdata[96]; // ASCII 32..126 is 96 chars
    std::string textureName;
    uint32_t textureIndex;
    float size;
};

class FontManager {
public:
    static FontManager* GetInstance();

    void Initialize();
    void Finalize();

    // Load a TTF font and bake an atlas for ASCII characters (32-126)
    void LoadFont(const std::string& fontName, const std::string& filePath, float pixelHeight);

    const CharacterInfo* GetCharacterInfo(const std::string& fontName) const;

private:
    FontManager() = default;
    ~FontManager() = default;
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    std::unordered_map<std::string, CharacterInfo> fonts_;
};
