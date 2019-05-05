struct VStoPS {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

SamplerState _sampler : register(s0);
Texture2D _source : register(t0);

// Found here
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 ACESFilm(float3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

float4 PS(VStoPS In) : SV_TARGET {
    return float4(ACESFilm(_source.Sample(_sampler, In.texcoord).rgb), 1);
}
