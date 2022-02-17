cbuffer PerFrameData : register(b0)
{
    float4 timeValues;

    float4x4 view;
    float4x4 projection;
    float4x4 model;
};

cbuffer LightData : register(b1)
{
    float4 lightDir;
}

struct VS_Input
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
    float3 norm : NORMAL0;
};

struct PS_Input
{
    float4 clipPos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 norm : NORMAL0;
};


PS_Input VS_Main(VS_Input input)
{
    PS_Input output = (PS_Input)0;
    output.clipPos = float4(input.pos, 1.0);
    output.clipPos = mul(model, output.clipPos);
    output.clipPos = mul(view, output.clipPos);
    output.clipPos = mul(projection, output.clipPos);

    output.uv = input.uv;

    output.norm = input.norm;

    return output;
}

float4 PS_Main(PS_Input input) : SV_TARGET
{
    float factor = dot(input.norm, lightDir.xyz);
    return float4(factor, factor, factor, 1.0f);
}
