#include "TextRenderer.h"
#include "FontManager.h"

TextRenderer* TextRenderer::GetInstance() {
    static TextRenderer instance;
    return &instance;
}

void TextRenderer::Initialize(SpriteCommon* spriteCommon) {
    spriteCommon_ = spriteCommon;
    usedSpriteCount_ = 0;
}

void TextRenderer::Finalize() {
    spritePool_.clear();
}

Sprite* TextRenderer::GetNextSprite() {
    if (usedSpriteCount_ >= spritePool_.size()) {
        auto sprite = std::make_unique<Sprite>();
        sprite->Initialize(spriteCommon_, "");
        spritePool_.push_back(std::move(sprite));
    }
    return spritePool_[usedSpriteCount_++].get();
}

void TextRenderer::Print(const std::string& fontName, const std::string& text, float x, float y, float size, const Vector4& color) {
    const CharacterInfo* info = FontManager::GetInstance()->GetCharacterInfo(fontName);
    if (!info) return;

    float scale = size / info->size;
    float startX = x;
    float startY = y;

    for (char c : text) {
        if (c >= 32 && c < 128) {
            const stbtt_bakedchar* b = &info->cdata[c - 32];
            
            // 四角形の生成
            float xpos = startX + b->xoff * scale;
            float ypos = startY + b->yoff * scale;
            float width = (b->x1 - b->x0) * scale;
            float height = (b->y1 - b->y0) * scale;

            Sprite* sprite = GetNextSprite();
            sprite->SetTextureIndex(info->textureIndex);
            
            // テクスチャのピクセル座標指定
            sprite->SetTextureLeftTop({(float)b->x0, (float)b->y0});
            sprite->SetTextureSize({(float)(b->x1 - b->x0), (float)(b->y1 - b->y0)});
            
            // 描画サイズと位置
            sprite->SetSize({width, height});
            sprite->SetPosition({xpos, ypos});
            sprite->SetAnchorPoint({0.0f, 0.0f}); // 左上基準
            sprite->SetColor(color);
            
            sprite->Update();

            // 次の文字へ進める
            startX += b->xadvance * scale;
        }
    }
}

void TextRenderer::Draw() {
    for (uint32_t i = 0; i < usedSpriteCount_; ++i) {
        spritePool_[i]->Draw();
    }
    usedSpriteCount_ = 0; // 次のフレーム用にリセット
}
