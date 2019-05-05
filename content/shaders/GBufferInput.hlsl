#ifndef __GBUFFER_INPUT_INCLUDED__
#define __GBUFFER_INPUT_INCLUDED__
SamplerState _samplerMain  : register(s0);
Texture2D _gbufferPosition : register(t0);
Texture2D _gbufferMaterial : register(t1);
Texture2D _gbufferColor : register(t2);
Texture2D _gbufferNormal : register(t3);

struct GBuffer {
  float3 position;
  float3 normal;
  
  float4 color;
  float specular;
  float metallic;
  float roughness;
};

GBuffer GetGBuffer(float2 texcoord) {
  GBuffer g;
  g.position = _gbufferPosition.Sample(_samplerMain, texcoord).rgb;
  g.normal = _gbufferNormal.Sample(_samplerMain, texcoord).rgb;
  g.color = _gbufferColor.Sample(_samplerMain, texcoord);

  float4 materialPacked = _gbufferMaterial.Sample(_samplerMain, texcoord);

  g.specular = materialPacked.r;
  g.metallic = materialPacked.g;
  g.roughness = materialPacked.b;

  return g;
}
#endif // __COMMON_INCLUDED__
