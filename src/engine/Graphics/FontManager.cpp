#include "FontManager.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../../external/stb/stb_truetype.h"

#include "TextureManager.h"
#include <fstream>
#include <cassert>
#include <vector>

FontManager* FontManager::GetInstance() {
    static FontManager instance;
    return &instance;
}

void FontManager::Initialize() {
}

void FontManager::Finalize() {
    fonts_.clear();
}

void FontManager::LoadFont(const std::string& fontName, const std::string& filePath, float pixelHeight) {
    if (fonts_.contains(fontName)) return;

    // TTFファイルをバイナリモードで読み込む
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    assert(file.is_open() && "Failed to open font file.");
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> ttfBuffer(size);
    if (!file.read((char*)ttfBuffer.data(), size)) {
        assert(false && "Failed to read font file.");
    }

    // フォントをベイクしてビットマップを生成
    int texWidth = 512;
    int texHeight = 512;
    std::vector<unsigned char> tempBitmap(texWidth * texHeight);
    
    CharacterInfo info;
    info.size = pixelHeight;
    info.textureName = fontName + "_Tex";

    // ASCII 32 から 96 文字分をベイク
    stbtt_BakeFontBitmap(ttfBuffer.data(), 0, pixelHeight, tempBitmap.data(), texWidth, texHeight, 32, 96, info.cdata);

    // stb_truetypeは8bitアルファのビットマップを生成するので、
    // DirectXTexで扱いやすい32bit RGBA (R8G8B8A8_UNORM) に変換する
    // R, G, B は 255(白) にして、Alpha値のみ適用する
    std::vector<uint32_t> rgbaBitmap(texWidth * texHeight);
    for (int i = 0; i < texWidth * texHeight; ++i) {
        uint8_t alpha = tempBitmap[i];
        // RGBA (Little-endian: A B G R)
        rgbaBitmap[i] = (alpha << 24) | (255 << 16) | (255 << 8) | 255;
    }

    // TextureManagerに直接ピクセルデータを渡してテクスチャをロード
    TextureManager::GetInstance()->LoadTextureFromRawPixels(
        info.textureName, texWidth, texHeight, DXGI_FORMAT_R8G8B8A8_UNORM, rgbaBitmap.data()
    );

    info.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(info.textureName);

    fonts_[fontName] = info;
}

const CharacterInfo* FontManager::GetCharacterInfo(const std::string& fontName) const {
    auto it = fonts_.find(fontName);
    if (it != fonts_.end()) {
        return &it->second;
    }
    return nullptr;
}
