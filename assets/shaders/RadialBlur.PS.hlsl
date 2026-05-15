#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer PostEffectParams : register(b0)
{
    int32_t kernelSize;
    float intensity;
    float dirX;
    float dirY;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    const float32_t2 kCenter = float32_t2(0.5f, 0.5f); //ブラーの中心
    int32_t kNumSamples = kernelSize;                  //サンプル数
    float32_t kBlurWidth = intensity;                  //ブラーの幅
    
    if (kNumSamples <= 0) kNumSamples = 1;
    
    float32_t2 direction = input.texcoord - kCenter;   //中心からの方向
    float32_t3 outputColor = float32_t3(0.0f, 0.0f, 0.0f);
    
    for (int32_t sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex)
    {
        float32_t2 texcoord = input.texcoord + direction * kBlurWidth * float32_t(sampleIndex);
        outputColor.rgb += gTexture.Sample(gSampler, texcoord).rgb;
    }
    outputColor.rgb *= rcp(kNumSamples); //平均化
    output.color.rgb = outputColor;
    output.color.a = 1.0f;               //アルファは1.0
    return output;
}