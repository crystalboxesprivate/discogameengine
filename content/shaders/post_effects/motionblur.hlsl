struct VStoPS {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

SamplerState _sampler : register(s0);
Texture2D _source : register(t0);
Texture2D _motionTexture : register(t1);

static int g_numSamples = 32;

float4 PS(VStoPS In) : SV_TARGET {
  float2 texCoord = In.texcoord;
  float4 color = _source.Sample(_sampler, texCoord);
  // float2 velocity = _motionTexture.Sample(_sampler, texCoord).xy * .5;
  float2 velocity = _motionTexture.Sample(_sampler, texCoord).xy * float2(1, -1) * .5;
  
  texCoord += velocity;
  for(int i = 1; i < g_numSamples; ++i, texCoord += velocity)
  {
    // Sample the color buffer along the velocity vector.
    float4 currentColor = _source.Sample(_sampler, texCoord);
    // Add the current color to our color sum.
    color += currentColor;
  }
  // Average all of the samples to get the final blur color.
  float4 finalColor = color / (float) g_numSamples;

  return finalColor;
}
