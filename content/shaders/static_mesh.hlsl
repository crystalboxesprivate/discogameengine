// position only hlsl // takes position from the vertex stream.
struct VertexInput {
  float4 position : POSITION;
};
// for shadow maps

// vertex stream begin
struct VertexInput {
  float4 position : POSITION;
  half2 texcoord: TEXCOORD;
  half3 normal : NORMAL;
  half3 tangent : BINORMAL;
  half4 color : COLOR;
};


//////// vertex stream end

// Shader common
// vertex stream common
struct VertexIntermediate {
  float4 position;
  half2 texcoord;
  half3 normal;
  half3 tangent;
  half4 color;
};

struct VertexOutput {
  float4 worldPosition;
  half2 worldNormal;
  half4 color;
};

VertexIntermediate GetIntermediate(VertexInput input) {
  // here we get the skinning calculated.
  return (VertexIntermediate) 0;
}


VertexOutput UserVertex(VertexIntermediate input) {
  return (VertexIntermediateToPS)input;
}

struct VSToPS {
  float4 pos : SV_POSITION;
  float3 normal : NORMAL;
  float2 texcoord : TEXCOORD;
  float4 col : COLOR;
};

VSToPS VSMain(VertexInput input) {
  VertexIntermediate intermediates = GetIntermediate(input);
  VertexOutput vertexOut = UserVertex(intermediates);
  // DO MVP stuff which goes to shaders
  return (VSToPS) 0;
}

struct PixelOut {
  float4 color;
  float roughness;
  float metal;
  float specular;
};

PixelOut UserPixel(VSToPS main) {
  return (PixelOut)0;
}

GBufferOut PSMain (VSToPS input) {
  PixelOut o = UserPixel(input)
  return (GBufferOut)0;
}

// GBufferCommin.hlsl
struct GBufferOut {
  float4 worldPos : SV_Target0;
  float4 materialAttributes : SV_Target1; // Spec, metal, roughness
  float4 color : SV_Target2; //
  float4 worldNormal : SV_Target3;
};
