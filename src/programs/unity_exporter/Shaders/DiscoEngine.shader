Shader "Custom/DiscoEngine"
{
    Properties {
        _Color ("Color", Color) = (1,1,1,1)
        _MainTex ("Albedo (RGB)", 2D) = "white" {}
        _OcclusionRoughnessMetallic ("Occlusion Roughness Metallic", 2D) = "white" {}
        _BumpMap ("Normal Map", 2D) = "white" {}
        
        _Roughness ("Roughness", Range(0,1)) = 0.0
        _Metallness ("Metallic", Range(0,1)) = 0.5

    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 200

        CGPROGRAM
        // Physically based Standard lighting model, and enable shadows on all light types
        #pragma surface surf Standard fullforwardshadows

        // Use shader model 3.0 target, to get nicer looking lighting
        #pragma target 3.0

        sampler2D _MainTex;
        sampler2D _OcclusionRoughnessMetallic;
        sampler2D _BumpMap;

        struct Input
        {
            float2 uv_MainTex;
        };

        fixed4 _Color;
        half _Roughness;
        half _Metallness;

        // Add instancing support for this shader. You need to check 'Enable Instancing' on materials that use the shader.
        // See https://docs.unity3d.com/Manual/GPUInstancing.html for more information about instancing.
        // #pragma instancing_options assumeuniformscaling
        UNITY_INSTANCING_BUFFER_START(Props)
            // put more per-instance properties here
        UNITY_INSTANCING_BUFFER_END(Props)

        void surf (Input IN, inout SurfaceOutputStandard o)
        {
            fixed4 orm = tex2D (_OcclusionRoughnessMetallic, IN.uv_MainTex) ;

            fixed4 c = tex2D (_MainTex, IN.uv_MainTex) * _Color * orm.r;
            fixed4 n = tex2D (_BumpMap, IN.uv_MainTex) ;

            float rough = orm.g * _Roughness;

            o.Albedo = c.rgb;
            // Metallic and smoothness come from slider variables
            o.Metallic = (_Metallness * orm.b);
            o.Smoothness =  (1.f - rough);
            o.Alpha = c.a;
            o.Normal = UnpackNormal (n);
        }
        ENDCG
    }
    FallBack "Diffuse"
}
