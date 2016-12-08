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
	float4 posW : PW;
	float4 texcoord : TEXCOORD;
	float4 normal : NORMAL;
};

PixelShaderInput main( VertexShaderInput input )
{
	float4 pos = float4( input.pos );
	float4 normal = float4( normalize( input.normal.xyz ), 1.0f );
	float4 texcoord = float4( input.texcoord );

	float4 posW = mul( pos, model );
	pos = mul( posW, view );
	pos = mul( pos, projection );
	normal = mul( normal, model );

	PixelShaderInput output;
	output.pos = pos;
	output.posW = posW;
	output.normal = float4( normalize( normal.xyz ), 1.0f );
	output.texcoord = texcoord;
	return output;
}