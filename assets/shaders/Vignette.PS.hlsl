#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer PostEffectParams : register(b0)
{
    int32_t kernelSize;   // 未使用
    float intensity;      // ビネットの強さ (1.0 = 通常, 大きいほど暗い)
    float dirX;           // 未使用
    float dirY;           // 未使用
    float time;           // 未使用
    float padding[3];
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);

    //周囲を0に、中心になるほど明るくなるように
    float32_t2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    float vignette = correct.x * correct.y * 16.0f;
    // intensity で pow の指数を制御: 大きい値ほどビネットが強くなる
    vignette = saturate(pow(vignette, intensity));
    output.color.rgb *= vignette;
    
    return output;
}