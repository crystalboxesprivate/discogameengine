#include "Cbuffer.hlsl"
#include "Common.hlsl"

struct Vert {
  float3 pos : POSITION;
};

struct VStoPS {
  float4 position :SV_POSITION;
  float3 texcoord : TEXCOORD0;
// #if MAKE_MOTION_VECTOR_PASS
//   float4 previous_pos : TEXCOORD1;
// #endif
};

struct SkyboxOut {
  float4 color : SV_TARGET0;

  // float4 velocity : SV_TARGET1;
};

float4x4 GetViewFixed(float4x3 inViewMatrix) {
  return float4x4 (
    inViewMatrix._m00, inViewMatrix._m01, inViewMatrix._m02, 0,
    inViewMatrix._m10, inViewMatrix._m11, inViewMatrix._m12, 0,
    inViewMatrix._m20, inViewMatrix._m21, inViewMatrix._m22, 0,
    0, 0, 0, 1
  );

}

VStoPS VS(Vert input) {
  float4x4 viewFixed = GetViewFixed(viewMatrix);

  float4x4 VP = mul(viewFixed, projectionMatrix);
  VStoPS o = (VStoPS)0;
  o.position = mul(float4(input.pos , 1.0), VP);
  o.texcoord = input.pos;

  // o.previous_pos = mul(float4(input.pos , 1.0), mul(GetViewFixed(viewMatrixPrevious), projectionMatrix));
  return o;
};

SamplerState samplerSkybox;
TextureCube skyboxTexture : register (t0);

SkyboxOut PS(VStoPS input) {
  SkyboxOut o;
  o.color = skyboxTexture.Sample(samplerSkybox, input.texcoord);
  // o.velocity = input.position - input.previous_pos;
  return o;
}

