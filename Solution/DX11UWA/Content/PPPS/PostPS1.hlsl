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
	float r = inColor.x;
	float g = inColor.y;
	float b = inColor.z;
	if ( r > 0.4f ) r = 1.0f;
	if ( g > 0.4f ) g = 1.0f;
	if ( b > 0.4f ) b = 1.0f;
	float4 outColor = float4( r, g, b, 1.0f );
	return outColor;
}