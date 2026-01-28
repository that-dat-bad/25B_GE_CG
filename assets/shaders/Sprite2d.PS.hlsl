#include "Sprite2d.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET;
};

PixelShaderOutput main(VSOutput input)
{
    PixelShaderOutput output;
    
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    output.color = textureColor * gMaterial.color;
    
    return output;
}
