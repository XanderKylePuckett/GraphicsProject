cbuffer ModelViewProjectionConstantBuffer : register( b0 )
{
	matrix model;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float4 pos : POSITION;
	float4 texcoord : TEXCOORD;
	float4 normal : NORMAL;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

PixelShaderInput main( VertexShaderInput input )
{
	PixelShaderInput output;
	output.pos = mul( input.pos, model );
	output.pos = mul( output.pos, view );
	output.pos = mul( output.pos, projection );
	output.color = float4( input.texcoord.xyz, 1.0f );
	return output;
}