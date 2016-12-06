texture2D tex : register( t0 );
SamplerState samplerstate : register( s0 );

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 posL : PL;
	float4 posW : PW;
	float4 texcoord : TEXCOORD;
	float4 normal : NORMAL;
};

float4 main( PixelShaderInput input ) : SV_TARGET
{
	float4 inColor = tex.Sample( samplerstate, input.texcoord.xy );
	float r = 1.0f - inColor.x;
	float g = 1.0f - inColor.y;
	float b = 1.0f - inColor.z;
	return float4( g, b, r, 1.0f );
}