#include "Common.hlsl"
#include "VertexType.generated.hlsl"

struct VSToPS {
  float4 pos : SV_POSITION;
};

VSToPS VSMain(VertexInput input) {
  VertexIntermediate intermediates = GetIntermediate(input);
  return (VSToPS) 0;
}

float PSMain (VSToPS input) : SV_TARGET {
  return 1.0;
}
