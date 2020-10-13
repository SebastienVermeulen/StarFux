/*
Firehaze.fx 
FirstName: Sebastien
LastName: Vermeulen 
Class: 2DAE_GD02
*/

//Matrix variables
float4x4 gWorldViewProj : WORLDVIEWPROJECTION;
float4x4 gWorld : WORLD;
float4x4 gMatrixViewInverse : VIEWINVERSE;
float4x4 gView : VIEW;

float gOffsetScale = 1.f;
float gHeat_Dist = 1.f;
float gTime = 0.f;

SamplerState samRenderTarget
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};
SamplerState samTexture
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

Texture2D gRenderTarget;
Texture3D gNoiseSource;

DepthStencilState depthStencilState
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

RasterizerState rasterizerState
{
    CullMode = BACK;
};

//BLENDSTATES
BlendState gDisableBlending
{
    BlendEnable[0] = TRUE;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD1;
};

//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT Out = (PS_INPUT)0;
    
    float width{};
    float height{};
    gRenderTarget.GetDimensions(width, height);
    
    // Output the position without transforming it
    // The Z value stays the same
    Out.Position = float4(input.Position.xyz, 1.f);
    // Texture coordinates are setup so that the full texture
    // is mapped completely onto the screen
    Out.TexCoord.x = 0.5f * (1.0f + input.Position.x + width);
    Out.TexCoord.y = 0.5f * (1.0f - input.Position.y + height);
    return Out;
}

//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
    float heatScale = 8.0f;
    float timeOffsetScale = 3.0f;

    // Read and scale the distortion offsets
    float2 offset = gNoiseSource.Sample(samTexture, float3(heatScale * input.TexCoord.x, heatScale * input.TexCoord.y + timeOffsetScale * gTime, gTime)).xy;
    offset = ((offset * 2.0) - 1.0) * gOffsetScale;
    
    // Offset texture lookup into our render target
    return gRenderTarget.Sample(samRenderTarget, input.TexCoord + offset);
}

//TECHNIQUE
//---------
technique11 FireHaze
{
    pass P0
    {
        SetRasterizerState(rasterizerState);
        SetDepthStencilState(depthStencilState, 0);
        SetBlendState(gDisableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f),	0xFFFFFFF);
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PS()));
    }
}