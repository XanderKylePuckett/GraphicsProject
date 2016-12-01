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
	float4 texcoord : TEXCOORD;
	float4 normal : NORMAL;
};

PixelShaderInput main( VertexShaderInput input )
{
	float4 pos = float4( input.pos );
	float4 normal = float4( input.normal );
	float4 texcoord = float4( input.texcoord );

	pos = mul( pos, model );
	pos = mul( pos, view );
	pos = mul( pos, projection );

	PixelShaderInput output;
	output.pos = pos;
	output.normal = normal;
	output.texcoord = texcoord;
	return output;
}