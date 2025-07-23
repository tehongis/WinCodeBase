// Vertex Shader
cbuffer ConstantBuffer : register(b0)
{
    matrix worldViewProj;
}

struct VS_INPUT {
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.tex = input.tex;
    return output;
}

// Pixel Shader
float4 ps_main(VS_OUTPUT input) : SV_TARGET {
    // Example: gradient based on texcoord
    return float4(input.tex.x, input.tex.y, 0.5f, 1.0f);
}
