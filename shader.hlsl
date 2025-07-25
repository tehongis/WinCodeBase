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
Texture2D bgTex : register(t0);
SamplerState samLinear : register(s0);

float4 ps_main(VS_OUTPUT input) : SV_TARGET {
    return bgTex.Sample(samLinear, input.tex);
}
