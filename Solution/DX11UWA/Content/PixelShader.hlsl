texture2D tex : register( t0 );
SamplerState samplerstate : register( s0 );

cbuffer LightingConstantBuffer : register( b0 )
{
	float4 dLightDirection;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 posL : PL;
	float4 texcoord : TEXCOORD;
	float4 normal : NORMAL;
};

float4 main( PixelShaderInput input ) : SV_TARGET
{
	float lightRatio = clamp( dot( -normalize( dLightDirection.xyz ), normalize( input.normal.xyz ) ), 0.0f, 1.0f );
	float4 lightColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
	float4 surfaceColor = tex.Sample( samplerstate, input.texcoord.xy );
	return lightRatio * lightColor * surfaceColor;
}