texture2D tex : register( t0 );
SamplerState samplerstate : register( s0 );

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 texcoord : TEXCOORD;
};

float4 main( PixelShaderInput input ) : SV_TARGET
{
	return tex.Sample( samplerstate, input.texcoord.xy );
}