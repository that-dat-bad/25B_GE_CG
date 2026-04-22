#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

static const float32_t2 kIndex3x3[9] =
{
    float32_t2(-1.0f, -1.0f),
    float32_t2(0.0f, -1.0f),
    float32_t2(1.0f, -1.0f),
    float32_t2(-1.0f, 0.0f),
    float32_t2(0.0f, 0.0f),
    float32_t2(1.0f, 0.0f),
    float32_t2(-1.0f, 1.0f),
    float32_t2(0.0f, 1.0f),
    float32_t2(1.0f, 1.0f)
};

static const float32_t kKernel3x3[3][3] =
{
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f }
};


struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    uint32_t width, height; //uvStepSizeの算出
    gTexture.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp(width), rcp(height));
    
    
    PixelShaderOutput output;
    output.color.rgb = float32_t3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    
    for (int32_t y = 0; y < 3; ++y)//3x3のカーネルを走査
    {
        for (int32_t x = 0; x < 3; ++x)
        {
            float32_t2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize; //現在のtexcoordを算出
            //色に1/9掛けて加算
            float32_t3 fetchColor = gTexture.Sample(gSampler, texcoord);
            output.color.rgb += fetchColor * kKernel3x3[x][y];
        }
    }
    return output;
}