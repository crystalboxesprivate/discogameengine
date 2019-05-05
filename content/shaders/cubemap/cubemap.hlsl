#include "VSToPS.hlsl"

cbuffer CubemapParameters {
  float4x4 projection;
  float4x4 view;
};

struct Vertex {
  float3 aPos : POSITION;
};


VStoPS VS(Vertex In) {
  VStoPS Out = (VStoPS)0;
  Out.position = mul(In.aPos, mul(view, projection));
  Out.position.zw = 1.0;
  // Out.worldPos = lerp(Out.worldPos, In.aPos, 1.0);
  return Out;
}
