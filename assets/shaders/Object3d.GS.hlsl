
// 頂点シェーダーからの入力
struct GSInput
{
    float32_t4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : WORLDPOSITION0;
};

// ピクセルシェーダーへの出力
struct GSOutput
{
    float32_t4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : WORLDPOSITION0;
};

// 変換行列を格納する定数バッファ (頂点シェーダーと同じ)
struct TransformationMatrix
{
    matrix WVP;
    matrix World;
    matrix WorldInverseTranspose;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

// ---------- パススルー（通常描画用） ----------
// 入力された三角形をそのまま出力する
[maxvertexcount(3)]
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> triStream)
{
    [unroll]
    for (uint i = 0; i < 3; i++)
    {
        GSOutput output;
        output.position = input[i].position;
        output.texcoord = input[i].texcoord;
        output.normal = input[i].normal;
        output.worldPosition = input[i].worldPosition;
        triStream.Append(output);
    }
    triStream.RestartStrip();
}
