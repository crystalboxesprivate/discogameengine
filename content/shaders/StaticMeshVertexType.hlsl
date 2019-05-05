#ifndef __STATIC_MESH_VERTEX_TYPE_INCLUDED__
#define __STATIC_MESH_VERTEX_TYPE_INCLUDED__

// vertex stream begin
struct VertexInput {
  float4 position : POSITION;
  half2 texcoord: TEXCOORD;
  half3 normal : NORMAL;
  half3 tangent : TANGENT;
  half4 color : COLOR;
};

VertexIntermediate GetIntermediate(VertexInput input) {
  // here we get the skinning calculated.
  float3 worldPos = mul(float4(input.position.xyz, 1.0), modelMatrix).xyz;
  float3 worldPosPrev = mul(float4(input.position.xyz, 1.0), modelMatrixPrev).xyz;

  VertexIntermediate o = (VertexIntermediate) 0;
  o.worldPosition = float4(worldPos, 1);
  o.worldPositionPrev  = float4(worldPosPrev, 1);

  o.worldNormal = mul(input.normal, (float3x3)modelMatrix);
  o.tangent = mul(input.tangent, (float3x3)modelMatrix); //input.tangent;


  o.texcoord = input.texcoord;
  o.color = input.color;
  return o;
}
#endif
