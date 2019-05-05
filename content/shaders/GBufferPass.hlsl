#include "Common.hlsl"
#include "VertexType.generated.hlsl"
#include "UserShader.generated.hlsl"

struct VSToPS {
  float4 posClipSpace : SV_POSITION;

  float4 col : COLOR;
  float3 normal : NORMAL;

  float3 worldSpacePos : TEXCOORD0;
  half2 texcoord : TEXCOORD1;

  float3 tangentToWorld0 : TEXCOORD2;
  float3 tangentToWorld2 : TEXCOORD3;

  float4 posClipSpacePrev : POSITION0;
  float4 posClipSpaceCurr : POSITION1;
  float4 timeConstant : TEXCOORD4;

};

struct GBufferOut {
  float4 worldPos : SV_Target0;
  float4 materialAttributes : SV_Target1; // Spec, metal, roughness
  float4 color : SV_Target2; //
  float4 worldNormal : SV_Target3;

#if MAKE_MOTION_VECTOR_PASS
  float4 motionVectors : SV_Target4;
#endif
};

VSToPS VSMain(VertexInput input) {
  VertexIntermediate intermediates = GetIntermediate(input);
  VertexOutput vertexOut = UserVertex(intermediates);
  
  VSToPS o;
  o.posClipSpace = mul(float4(vertexOut.worldPosition, 1.0), GetVP());
  o.normal = vertexOut.worldNormal;

  o.tangentToWorld0 = intermediates.tangent;
  o.tangentToWorld2 = intermediates.worldNormal;

  o.col = vertexOut.color;
  o.worldSpacePos = vertexOut.worldPosition.xyz;

  o.texcoord = intermediates.texcoord;

  o.posClipSpacePrev = mul(intermediates.worldPositionPrev, GetPrevVP());
  o.posClipSpaceCurr = o.posClipSpace;

  o.posClipSpacePrev.xyz /= max(1, o.posClipSpacePrev.w);
  o.posClipSpaceCurr.xyz /= max(1,o.posClipSpaceCurr.w);
  o.timeConstant = _timeConstant;
  return o;
}

half3x3 makeTangentToWorldMatrix(half3 tangentToWorld0, half3 tangentToWorld2) {
  return half3x3(tangentToWorld0, cross(tangentToWorld2, tangentToWorld0), tangentToWorld2);
}

GBufferOut PSMain (VSToPS input) {
  PixelInput px = (PixelInput)0;
  px.pos = input.worldSpacePos;
  px.normal = input.normal;
  px.texcoord = input.texcoord;
  px.col = input.col;

  px.tangentToWorld = makeTangentToWorldMatrix(input.tangentToWorld0, input.tangentToWorld2);

  PixelOut pixelOut = UserPixel(px);

  GBufferOut o = (GBufferOut)0;
  o.worldPos = float4(input.worldSpacePos, 1);
  o.materialAttributes = float4(pixelOut.specular, pixelOut.metal, pixelOut.roughness, 1);
  o.color = pixelOut.color;
  o.worldNormal = float4(normalize(pixelOut.worldNormal),1);

#if MAKE_MOTION_VECTOR_PASS
  // o.motionVectors = input.velocity;
  o.motionVectors = float4((input.posClipSpaceCurr.xyz  - input.posClipSpacePrev.xyz ) / input.timeConstant.y * (1/1920.0), 1.0);
#endif
  return o;
}
