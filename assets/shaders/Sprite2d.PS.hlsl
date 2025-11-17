#include "Sprite2d.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET;
};

PixelShaderOutput main(VSOutput input)
{
    PixelShaderOutput output;
    
    // テクスチャから色を取り出す
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    // マテリアル色を乗算して出力
    output.color = textureColor * gMaterial.color;
    
    return output;
}