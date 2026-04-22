#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer BoxFilterParams : register(b0)
{
    int32_t kernelSize;
    float3 padding;
};


struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    uint32_t width, height; //uvStepSizeの算出
    gTexture.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp((float32_t)width), rcp((float32_t)height));
    
    
    PixelShaderOutput output;
    output.color.rgb = float32_t3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    
    int32_t halfSize = kernelSize / 2;
    int32_t startOffset = -halfSize;
    int32_t endOffset = kernelSize % 2 == 0 ? halfSize - 1 : halfSize;
    
    float32_t weight = 1.0f / (float32_t)(kernelSize * kernelSize);
    
    for (int32_t y = startOffset; y <= endOffset; ++y)
    {
        for (int32_t x = startOffset; x <= endOffset; ++x)
        {
            float32_t2 texcoord = input.texcoord + float32_t2(x, y) * uvStepSize; //現在のtexcoordを算出
            //色に重みを掛けて加算
            float32_t3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            output.color.rgb += fetchColor * weight;
        }
    }
    return output;
}