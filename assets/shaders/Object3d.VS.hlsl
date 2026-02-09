struct VertexInput
{
    float32_t4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

struct VertexOutput
{
    float32_t4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : WORLDPOSITION0;
};

struct TransformationMatrix
{
    matrix WVP; // World * View * Projection
    matrix World; // World
    matrix WorldInverseTranspose; // WorldInverseTranspose for non-uniform scaling
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    output.position = mul(input.position, gTransformationMatrix.WVP);
    
    output.texcoord = input.texcoord;
    
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix.WorldInverseTranspose));
    
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    
    return output;
}

