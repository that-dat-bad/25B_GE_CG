#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer PostEffectParams : register(b0)
{
    int32_t kernelSize;   // 未使用
    float intensity;      // ビネットの強さ (1.0 = 通常, 大きいほど暗い)
    float dirX;           // 未使用
    float dirY;           // 未使用
    float time;           // 未使用
    float padding[3];
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);

    //周囲を0に、中心になるほど明るくなるように
    float32_t2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    float vignette = correct.x * correct.y * 16.0f;
    
    // 1. ブラックアウト処理
    // intensity で pow の指数を制御: 大きい値ほどビネット（暗転）が強くなる
    float blackoutFactor = saturate(pow(vignette, intensity));
    output.color.rgb *= blackoutFactor;

    // 2. レッドアウト処理
    // dirX はレッドアウトの強度（0.0〜）
    if (dirX > 0.0f) {
        // 画面の端ほど強く赤くなり、中央も赤みがかかるようにする
        // (1.0 - vignette) は画面端で 1.0、中央で 0.0 になる
        float edgeRedness = saturate(1.0f - vignette);
        
        // レッドアウトの強さに応じて、基本の赤み＋端の赤みをブレンド
        // 全体を赤く染めつつ、端はさらに濃い赤にする
        float totalRedness = saturate(dirX * (0.3f + edgeRedness * 0.7f));
        
        float3 redColor = float3(0.9f, 0.05f, 0.05f); // 血のような赤
        output.color.rgb = lerp(output.color.rgb, redColor, totalRedness);
    }
    
    return output;
}