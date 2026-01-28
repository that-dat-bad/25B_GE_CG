#include "Particle.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET;
};

PixelShaderOutput main(VSOutput input)
{
    PixelShaderOutput output;

    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    output.color = textureColor * input.color;
    
    if (output.color.a == 0.0f)
    {
        discard;
    }

    return output;
}
