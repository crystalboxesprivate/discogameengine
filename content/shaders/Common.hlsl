#ifndef __COMMON_INCLUDED__
#define __COMMON_INCLUDED__
#include "Cbuffer.hlsl"

struct VertexOutput {
  float3 worldPosition;
  half3 worldNormal;
  half4 color;
};

struct VertexIntermediate {
  float4 worldPosition;
  float4 worldPositionPrev;
  
  half3 worldNormal;

  half2 texcoord;
  half3 tangent;
  half4 color;
};

struct PixelOut {
  float4 color;
  float roughness;
  float metal;
  float specular;
  half3 worldNormal;
};

struct PixelInput {
  float3 pos;
  float3 normal;
  float2 texcoord;
  float4 col;

  half3x3 tangentToWorld;
};

inline float4x4 GetVP() {
  return mul(viewMatrix, projectionMatrix);
}

inline float4x4 GetPrevVP() {
  return mul(viewMatrixPrevious, projectionMatrix);
}

inline half3 LinearToGammaSpace (half3 linRGB)
{
    linRGB = max(linRGB, half3(0.h, 0.h, 0.h));
    // An almost-perfect approximation from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return max(1.055h * pow(linRGB, 0.416666667h) - 0.055h, 0.h);

    // Exact version, useful for debugging.
    //return half3(LinearToGammaSpaceExact(linRGB.r), LinearToGammaSpaceExact(linRGB.g), LinearToGammaSpaceExact(linRGB.b));
}

inline half3 GammaToLinearSpace (half3 sRGB)
{
    // Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return sRGB * (sRGB * (sRGB * 0.305306011h + 0.682171111h) + 0.012522878h);

    // Precise version, useful for debugging.
    //return half3(GammaToLinearSpaceExact(sRGB.r), GammaToLinearSpaceExact(sRGB.g), GammaToLinearSpaceExact(sRGB.b));
}

#endif // __COMMON_INCLUDED__
