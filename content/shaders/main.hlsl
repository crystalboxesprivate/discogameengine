cbuffer cbPerObject
{
  float4x4 model;
  float4x4 view;
  float4x4 projection;
};

struct Vertex {
  float3 pos : POSITION;
  float2 texcoord : TEXCOORD;
  float3 normal : NORMAL;
  float3 col : COLOR;
};

struct VStoPS {
  float4 pos : SV_POSITION;
  float4 col : COLOR;
  float3 normal : NORMAL;
};

inline float4x4 GetVP() {
  return mul(view, projection);
}

inline float4x4 GetMVP() {
  return mul(model, GetVP());
}

inline float3 GetEyeVec() {
  return -transpose(view)._m20_m21_m22;
}

VStoPS VS(Vertex input) 
{
  VStoPS o = (VStoPS)0;
 #if 0
  float4x4 MVP = mul(view, projection);
  MVP = mul(model, MVP);
  o.pos = mul(float4(input.pos, 1.0), MVP );
#else
  float3 worldPos = mul(float4(input.pos, 1.0), model).xyz;
  o.pos = mul(float4(worldPos, 1.0), GetVP());
#endif
  float4x4 viewTransp = transpose(view);
  float3 eye = -viewTransp._m20_m21_m22;

  o.col = float4(eye, 1.0);
  o.normal = mul(float4(input.normal, 0), model).xyz;
  return o;
}

float4 PS(VStoPS input) : SV_TARGET
{
  float s = saturate((float3) dot(input.col.rgb, input.normal)) + 0.2;
  return  float4(max(saturate(input.normal), s * 0.7) , 1); 
}

