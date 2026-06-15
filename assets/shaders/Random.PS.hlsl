#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

struct PostEffectParams {
    int32_t kernelSize;
    float intensity;
    float dirX;
    float dirY;
    float time;
};

ConstantBuffer<PostEffectParams> gParams : register(b0);

// from https://qiita.com/Kashiwabara_Aki/items/e83fbde26c2e276f7a63
float rand2dTo1d(float2 value, float2 dotDir = float2(12.9898, 78.233)) {
    float2 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // timeを利用して乱数を生成
    float32_t random = rand2dTo1d(input.texcoord * gParams.time);
    
    // もとの画像と乗算する (Slide 11対応)
    float32_t4 baseColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = baseColor * random;
    output.color.a = baseColor.a; // alphaは元のままにする
    
    return output;
}
