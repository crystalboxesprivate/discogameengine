#include "GBufferInput.hlsl"

cbuffer LightPassParams {
  float3 viewPos;
  float3 lightDirection;
  float3 lightPosition;
  float3 lightColor;
  float4 lightParameters; // linear quadratic radius
};

struct VStoPS {
    float4 position : SV_POSITION;
	  float2 uv : TEXCOORD;
};

float4 PS(VStoPS In) : SV_TARGET
{
  GBuffer gbuffer = GetGBuffer(In.uv);

  // then calculate lighting as usual
  float3 lighting  = gbuffer.color.rgb * 0.001; // hard-coded ambient component
  float3 viewDir  = normalize(viewPos - gbuffer.position);

  lighting += max(dot(gbuffer.normal, lightDirection), 0.0) * gbuffer.color.rgb * 1.0;

  return float4(lighting, gbuffer.color.a);
}
