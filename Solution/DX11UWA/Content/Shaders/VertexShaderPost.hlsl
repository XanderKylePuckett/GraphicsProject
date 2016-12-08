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
};

PixelShaderInput main( VertexShaderInput input )
{
	PixelShaderInput output;
	output.pos = input.pos;
	output.texcoord = input.texcoord;
	return output;
}