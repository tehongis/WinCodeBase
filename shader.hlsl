// Vertex Shader
cbuffer ConstantBuffer : register(b0)
{
    matrix worldViewProj;
}

struct VS_INPUT {
    float3 pos : POSITION;
};

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    return output;
}

// Pixel Shader
float4 ps_main() : SV_TARGET {
    return float4(1.0f, 0.4f, 0.2f, 1.0f); // Orange tint
}
