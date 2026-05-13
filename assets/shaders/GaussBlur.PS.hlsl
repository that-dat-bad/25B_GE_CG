#include "FullScreen.hlsli"

static const float32_t PI = 3.1415926535897932384626433832795f;

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer PostEffectParams : register(b0)
{
    int32_t kernelSize;
    float intensity; // sigma
    float2 padding;
};
float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) / (2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    uint32_t width, height;
    gTexture.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp((float32_t)width), rcp((float32_t)height));

    float32_t weight = 0.0f;
    float32_t4 color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    
    int32_t halfSize = kernelSize / 2;
    int32_t startOffset = -halfSize;
    int32_t endOffset = kernelSize % 2 == 0 ? halfSize - 1 : halfSize;

    for (int32_t y = startOffset; y <= endOffset; ++y)
    {
        for (int32_t x = startOffset; x <= endOffset; ++x)
        {
            float w = gauss(float(x), float(y), intensity);
            weight += w;
            
            float32_t2 texcoord = input.texcoord + float32_t2(x, y) * uvStepSize;
            float32_t3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            color.rgb += fetchColor * w;
        }
    }
    
    PixelShaderOutput output;
    output.color.rgb = color.rgb * rcp(weight);
    output.color.a = 1.0f;

    return output;
}