#include "GBufferInput.hlsl"

#define PI 3.14159265359

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

Texture2D _brdfLUT : register(t4);

SamplerState _cubemapSampler : register(s1);
TextureCube _skyColorMap : register(t5);
TextureCube _skyIrradiance : register(t6);
TextureCube _skyPrefilter : register(t7);

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max((float3)(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

float4 PS(VStoPS In) : SV_TARGET
{
  GBuffer gbuffer = GetGBuffer(In.uv);
#if 1
  float3 albedo = gbuffer.color.rgb;
  float metallic = gbuffer.metallic;
  float roughness = gbuffer.roughness;
#else
  float3 albedo = float3(0.96, 0, 0);
  float metallic = 1.0;
  float roughness = 0.0;
#endif
  float3 N = gbuffer.normal;
  float3 V = normalize(viewPos - gbuffer.position);
  float3 R = reflect(-V, N); 

  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
  // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
  float3 F0 = (float3)0.04; 
  F0 = lerp(F0, albedo, metallic);
  
  // ambient lighting (we now use IBL as the ambient term)
  float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

  float3 kS = F;
  float3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;

  float3 irradiance = _skyIrradiance.Sample(_cubemapSampler, N).rgb;
  float3 diffuse = irradiance * albedo;

  float MAX_REFLECTION_LOD = 4.0;
    
  float3 prefilteredColor = _skyPrefilter.SampleLevel(_cubemapSampler, R,  roughness * MAX_REFLECTION_LOD).rgb;    
  float2 brdf_uv = float2(max(dot(N, V), 0.0), roughness);
  float2 brdf  = _brdfLUT.Sample(_samplerMain, float2(brdf_uv.x, 1.0 - brdf_uv.y)).rg;
  float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

  float3 ambient = (kD * diffuse + specular);// * ao;
  float3 color = ambient;// + Lo;

  return float4(color, gbuffer.color.a);
}
