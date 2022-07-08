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

Texture2D shadowmap: register(t0);
sampler shadowSampler: register(s0);

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
    float4 lightViewPos : TEXCOORD1;
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

    output.lightViewPos = float4(input.pos, 1.0);
    output.lightViewPos = mul(model, output.clipPos);
    output.lightViewPos = mul(objectToLight, output.clipPos);

    return output;
}

float4 PS_Main(PS_Input input) : SV_TARGET
{
    // Testing shadow sample
    float depth = shadowmap.Sample(shadowSampler, input.uv);

    float factor = dot(input.norm, lightDir.xyz); // Light influence
    factor += 0.4; // Ambient factor

    return float4(factor, factor, factor, 1.0f);
}
