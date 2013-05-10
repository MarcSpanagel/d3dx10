// FX für einen einfachen Würfel

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
matrix World;
matrix View;
matrix Projection;
matrix WVP;

Texture2D DiffuseMap;

SamplerState TriLinearSample
{
	Filter = MIN_MAG_MIP_LINEAR;
};

struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul( input.Pos, WVP );
    /*output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );*/
    output.Tex = input.Tex;
    return output;
}

float4 PS( PS_INPUT input) : SV_Target
{
    float4 diffuse = DiffuseMap.Sample( TriLinearSample, input.Tex );
    return diffuse;    // Set the color of the pixel to the corresponding texel in the loaded image is.
}



technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
        //SetRasterizerState(Wireframe);
    }
}