struct VS_Input
{
    float3 pos : POSITION;
};

struct PS_Input
{
    float4 clipPos : SV_POSITION;
};


PS_Input VS_Main(VS_Input input)
{
    PS_Input output = (PS_Input)0;
    output.clipPos = float4(input.pos, 1.0);

    return output;
}

float4 PS_Main(PS_Input input) : SV_TARGET
{
    return float4(0.0, 1.0, 1.0, 1.0);
}
