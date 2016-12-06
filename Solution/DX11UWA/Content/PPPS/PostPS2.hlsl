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
	float av = ( inColor.x + inColor.y + inColor.z ) / 3.0f;
	av = clamp( av * 1.4f, 0.0f, 1.0f );
	return float4( av, av, av, 1.0f );
}