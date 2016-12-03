textureCUBE tex : register( t0 );
SamplerState samplerstate : register( s0 );

cbuffer LightingConstantBuffer : register( b0 )
{
	float4 lightState;
	float4 dLightDirection;
	float4 pLightPos0;
	float4 pLightPos1;
	float4 pLightPos2;
	float4 pLightColorRadius0;
	float4 pLightColorRadius1;
	float4 pLightColorRadius2;
};

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
	float4 result = tex.Sample( samplerstate, input.posL.xyz );
	if ( lightState.w == 1.0f ) result = float4( result.x, result.x * 0.95f, result.x * 0.95f, 1.0f );
	else if ( lightState.w == 2.0f ) result = float4( result.y * 0.95f, result.y, result.y * 0.95f, 1.0f );
	else if ( lightState.w == 3.0f ) result = float4( result.z * 0.95f, result.z * 0.95f, result.z, 1.0f );
	else if ( lightState.w == 4.0f )
	{
		float color = ( result.x + result.y + result.z ) / 3.0f;
		result = float4( color, color, color, 1.0f );
	}
	return result;
}