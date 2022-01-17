cbuffer PerFrameData : register(b0)
{
    float4 timeValues;
};

struct VS_Input
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_Input
{
    float4 clipPos : SV_POSITION;
    float2 uv : TEXCOORD0;
};


PS_Input VS_Main(VS_Input input)
{
    PS_Input output = (PS_Input)0;
    output.clipPos = float4(input.pos, 1.0);
    output.uv = timeValues.xx * input.uv;

    return output;
}

float4 PS_Main(PS_Input input) : SV_TARGET
{
    return float4(frac(input.uv.x), frac(input.uv.y), 0.0, 1.0);
}
