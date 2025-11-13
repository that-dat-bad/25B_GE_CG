#include "Sprite2d.hlsli"

struct VSInput
{
    float32_t4 position : POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.svpos = mul(input.position, gMaterial.WVP);
    output.texcoord = input.texcoord;
    return output;
}