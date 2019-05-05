struct Vertex {
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VStoPS {
    float4 Position : SV_POSITION;
	  float2 TexCoord : TEXCOORD;
};

Texture2D Tex;
SamplerState TexSamplerState;

VStoPS VS(Vertex In) 
{
    VStoPS Out = (VStoPS)0;
    Out.Position = float4(In.Position, 1);
    Out.TexCoord = In.TexCoord;
    return Out;
}

float4 PS(VStoPS In) : SV_TARGET {
    return float4(Tex.Sample(TexSamplerState, In.TexCoord).rgb, 1);
}
