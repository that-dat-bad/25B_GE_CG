#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../../../external/stb/stb_truetype.h"
#include "../base/Math/MyMath.h"

using namespace MyMath;

struct CharacterInfo {
    std::unordered_map<char32_t, stbtt_packedchar> glyphs;
    std::string textureName;
    uint32_t textureIndex;
    float size;
};

class FontManager {
public:
    static FontManager* GetInstance();

    void Initialize();
    void Finalize();

    void LoadFont(const std::string& fontName, const std::string& filePath, float pixelHeight);

    const CharacterInfo* GetCharacterInfo(const std::string& fontName) const;

private:
    FontManager() = default;
    ~FontManager() = default;
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    std::unordered_map<std::string, CharacterInfo> fonts_;
};
