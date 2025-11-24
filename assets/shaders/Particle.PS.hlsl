#include "Particle.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET;
};

PixelShaderOutput main(VSOutput input)
{
    PixelShaderOutput output;

    // テクスチャから色を取り出す
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    // テクスチャの色と、頂点シェーダーから送られてきたパーティクルの色を乗算
    output.color = textureColor * input.color;
    
    // アルファ値が0なら棄却 (必要に応じて)
    if (output.color.a == 0.0f)
    {
        discard;
    }

    return output;
}