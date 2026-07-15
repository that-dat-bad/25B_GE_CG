#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PostEffectParams {
    int32_t kernelSize;
    float intensity;      // 光量増幅倍率 (Exposure / Gain multiplier)
    float dirX;
    float dirY;
    float time;
    float3 colorTint;
};
ConstantBuffer<PostEffectParams> gParams : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 baseColor = gTexture.Sample(gSampler, input.texcoord);
    
    // RGBの光量を intensity (倍率) によって増幅する
    float32_t3 finalColor = baseColor.rgb * gParams.intensity;
    
    output.color = float32_t4(finalColor, baseColor.a);
    return output;
}
