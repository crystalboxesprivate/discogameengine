struct VStoPS {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

SamplerState _sampler : register(s0);
Texture2D _source : register(t0);

float4 PS(VStoPS In) : SV_TARGET {
    return float4(_source.Sample(_sampler, In.texcoord).rgb, 1);
}
