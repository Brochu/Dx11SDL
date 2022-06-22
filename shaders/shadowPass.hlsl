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
    float4x4 objectToLight;
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
};


PS_Input VS_Main_Shadow(VS_Input input)
{
    PS_Input output = (PS_Input)0;
    output.clipPos = float4(input.pos, 1.0);
    output.clipPos = mul(model, output.clipPos);
    output.clipPos = mul(objectToLight, output.clipPos);

    return output;
}
