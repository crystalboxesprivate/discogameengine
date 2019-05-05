// vertex stream begin
struct VertexInput {
  float4 position : POSITION;
  half4 normal : NORMAL;
  half4 texcoord: TEXCOORD0;
  half4 tangent : TANGENT;
  half4 binormal : BINORMAL;
  float4 boneId : TEXCOORD1;
  float4 boneWeights : TEXCOORD2;
};

#define MAXNUMBEROFBONES 100
cbuffer BonesBuffer : register(b2)
{
  float4x4 bones[MAXNUMBEROFBONES];
  float4x4 bonesPrevious[MAXNUMBEROFBONES];
  // float4 numBonesUsed;
};

#include "utils/Matrix.hlsl"

VertexIntermediate GetIntermediate(VertexInput input) {

  
	float3 posTemp = input.position.xyz;

  float4x4 boneTransform 
                  = bones[(int)input.boneId.x] * input.boneWeights.x;
  boneTransform += bones[(int)input.boneId.y] * input.boneWeights.y;
  boneTransform += bones[(int)input.boneId.z] * input.boneWeights.z;
  boneTransform += bones[(int)input.boneId.w] * input.boneWeights.w;

    float4x4 boneTransformPrev
                  = bonesPrevious[(int)input.boneId.x] * input.boneWeights.x;
  boneTransformPrev += bonesPrevious[(int)input.boneId.y] * input.boneWeights.y;
  boneTransformPrev += bonesPrevious[(int)input.boneId.z] * input.boneWeights.z;
  boneTransformPrev += bonesPrevious[(int)input.boneId.w] * input.boneWeights.w;
 
  float3 vertPositionFromBones = mul(float4(posTemp.xyz, 1.0), boneTransform).xyz;
  float3 vertPositionFromBonesPrev = mul(float4(posTemp.xyz, 1.0), boneTransformPrev).xyz;


  // here we get the skinning calculated.
  float3 worldPos = mul(float4(vertPositionFromBones, 1.0), modelMatrix).xyz;
  float3 worldPosPrev = mul(float4(vertPositionFromBonesPrev, 1.0), modelMatrixPrev).xyz;

  float4x4 matBoneTransform_InTrans = inverse(transpose(boneTransform));
  float3 vNormBone = mul(input.normal.xyz, (float3x3)matBoneTransform_InTrans); //matBoneTransform_InTrans * vec4(normalize(vNormal.xyz),1.0f);
  float3 vTanXYZ_Bone = mul(input.tangent.xyz, (float3x3)matBoneTransform_InTrans); //matBoneTransform_InTrans * vec4(normalize(vNormal.xyz),1.0f);
  VertexIntermediate o = (VertexIntermediate) 0;

  o.worldNormal = mul(vNormBone, (float3x3)modelMatrix);
  o.tangent = mul(vTanXYZ_Bone, (float3x3)modelMatrix);


  o.worldPosition = float4(worldPos, 1);
  o.worldPositionPrev  = float4(worldPosPrev, 1);





  o.texcoord = input.texcoord.xy;
  o.color = half4(1,1,1,1);

  return o;
}
