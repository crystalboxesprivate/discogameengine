#include "Cbuffer.hlsl"
#include "utils/Matrix.hlsl"

struct Vertex {
  float4 position : POSITION;
  half2 texcoord: TEXCOORD;
  half3 normal : NORMAL;
  half3 tangent : TANGENT;
  half4 color : COLOR;
};

struct VStoPS {
  float4 posClipSpace : SV_POSITION;
  float4 col : COLOR;
  float3 normal : NORMAL;
  float3 worldSpacePos : TEXCOORD0;
  float3 velocity : TEXCOORD1;
};

inline float4x4 GetVP() {
  return mul(viewMatrix, projectionMatrix);
}
inline float3 GetEyeVec() {
  return -transpose(viewMatrix)._m20_m21_m22;
}

VStoPS VS(Vertex input) 
{
  VStoPS o = (VStoPS)0;
  float3 worldPos = mul(float4(input.position.xyz, 1.0), modelMatrix).xyz;
  float3 worldPosPrev = mul(float4(input.position.xyz, 1.0), modelMatrixPrev).xyz;

  o.posClipSpace = mul(float4(worldPos, 1.0), GetVP());
  o.normal = mul(input.normal, (float3x3)modelMatrix);

  o.col = input.color;
  o.worldSpacePos = worldPos;
  o.velocity = worldPos - worldPosPrev;

  return o;
}

struct GBufferOut {
  float4 worldPos : SV_Target0;
  float4 materialAttributes : SV_Target1; // Spec, metal, roughness
  float4 color : SV_Target2; //
  float4 worldNormal : SV_Target3;
};

GBufferOut PS(VStoPS input) {
  float s = saturate((float3) dot(input.col.rgb, input.normal)) + 0.2;
  float4 color = (float4)1;// float4(max(saturate(input.normal), s * 0.7) , 1); 
  GBufferOut o = (GBufferOut)0;

  o.worldPos.xyz = input.worldSpacePos;
  o.materialAttributes = float4(1, 0, 0.5, 1);
  o.color = color;
  o.worldNormal.xyz = normalize(input.normal);

  o.worldPos.w = input.velocity.x;
  o.materialAttributes.w = input.velocity.y;
  o.worldNormal.w = input.velocity.z;

  return o;
}

