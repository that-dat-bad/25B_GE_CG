#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
Texture2D<float32_t> gDepthTexture : register(t1);

SamplerState gSamplerLinear : register(s0);
SamplerState gSamplerPoint : register(s1);

cbuffer PostEffectParams : register(b0)
{
    int32_t kernelSize;
    float intensity;
    float dirX;
    float dirY;
    float32_t4x4 projectionInverse;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

static const float32_t kPrewittHorizontalKernel[3][3] = {
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
};

static const float32_t kPrewittVerticalKernel[3][3] = {
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    {  0.0f,         0.0f,         0.0f },
    {  1.0f / 6.0f,  1.0f / 6.0f,  1.0f / 6.0f },
};

PixelShaderOutput main(VertexShaderOutput input)
{
    uint32_t width, height;
    gTexture.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp((float32_t)width), rcp((float32_t)height));

    float32_t2 difference = float32_t2(0.0f, 0.0f);
    
    for (int32_t y = 0; y < 3; ++y) {
        for (int32_t x = 0; x < 3; ++x) {
            float32_t2 texcoord = input.texcoord + float32_t2(x - 1, y - 1) * uvStepSize;
            float32_t ndcDepth = gDepthTexture.Sample(gSamplerPoint, texcoord);
            
            float32_t4 viewSpace = mul(float32_t4(0.0f, 0.0f, ndcDepth, 1.0f), projectionInverse);
            float32_t viewZ = viewSpace.z * rcp(viewSpace.w);

            difference.x += viewZ * kPrewittHorizontalKernel[y][x];
            difference.y += viewZ * kPrewittVerticalKernel[y][x];
        }
    }

    float32_t weight = length(difference);
    weight = saturate(weight * 6.0f); // Adjust multiplier as needed based on scene scale

    PixelShaderOutput output;
    output.color.rgb = (1.0f - weight) * gTexture.Sample(gSamplerLinear, input.texcoord).rgb;
    output.color.a = 1.0f;
    return output;
}
