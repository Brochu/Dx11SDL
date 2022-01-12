struct vs_in
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 norm : NORMAL;
};

struct vs_out
{
    float4 clipPos : SV_POSITION;
};


vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out)0;
    output.clipPos = float4(input.pos, 1.0);

    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    return float4(1.0, 0.0, 1.0, 1.0);
}
