#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    const float32_t2 kCenter = float32_t2(0.5f, 0.5f); //ブラーの中心
    const int32_t kNumSamples = 16;                    //サンプル数
    const float32_t kBlurWidth = 0.5f;                 //ブラーの幅
    
    float32_t2 direction = input.texcoord - kCenter; //中心からの方向
    
    output.color = gTexture.Sample(gSampler, input.texcoord);
    return output;
}