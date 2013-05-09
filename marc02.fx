// FX für einen einfachen Würfel

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
matrix World;
matrix View;
matrix Projection;
matrix WVP;

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul( input.Pos, WVP );
    /*output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );*/
    output.Color = input.Color;
    return output;
}

float4 PS( PS_INPUT input) : SV_Target
{
    return input.Color;
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