texture2D tex : register( t0 );
SamplerState samplerstate : register( s0 );

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 texcoord : TEXCOORD;
};

float4 main( PixelShaderInput input ) : SV_TARGET
{
	float4 inColor = tex.Sample( samplerstate, input.texcoord.xy );
	float r = inColor.x;
	float g = inColor.y;
	float b = inColor.z;
	if ( r > 0.5f ) r = 1.0f; else if ( r > 0.35f ) r = 0.8f;
	if ( g > 0.5f ) g = 1.0f; else if ( g > 0.35f ) g = 0.8f;
	if ( b > 0.5f ) b = 1.0f; else if ( b > 0.35f ) b = 0.8f;
	float4 outColor = float4( r, g, b, 1.0f );
	return outColor;
}