texture2D tex : register( t0 );
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
	float4 surfaceColor = tex.Sample( samplerstate, input.texcoord.xy );
	float4 result = float4( 0.0f, 0.0f, 0.0f, 1.0f );
	if ( lightState.x <= 0.5f && lightState.y <= 0.5f )
		result = surfaceColor;
	else
	{
		if ( lightState.x > 0.5f )
		{
			float lightRatio = clamp( dot( -normalize( dLightDirection.xyz ), normalize( input.normal.xyz ) ), 0.0f, 1.0f );
			float4 lightColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
			result = lightRatio * lightColor * surfaceColor;
		}
		if ( lightState.y > 0.5f )
		{
			float3 lightDir0 = normalize( pLightPos0.xyz - input.posW.xyz );
			float4 lightColor0 = float4( pLightColorRadius0.xyz, 1.0f );
			float lightRatio0 = clamp( dot( lightDir0, normalize( input.normal.xyz ) ), 0.0f, 1.0f );
			float4 result0 = lightRatio0 * lightColor0 * surfaceColor;

			float3 lightDir1 = normalize( pLightPos1.xyz - input.posW.xyz );
			float4 lightColor1 = float4( pLightColorRadius1.xyz, 1.0f );
			float lightRatio1 = clamp( dot( lightDir1, normalize( input.normal.xyz ) ), 0.0f, 1.0f );
			float4 result1 = lightRatio1 * lightColor1 * surfaceColor;

			float3 lightDir2 = normalize( pLightPos2.xyz - input.posW.xyz );
			float4 lightColor2 = float4( pLightColorRadius2.xyz, 1.0f );
			float lightRatio2 = clamp( dot( lightDir2, normalize( input.normal.xyz ) ), 0.0f, 1.0f );
			float4 result2 = lightRatio2 * lightColor2 * surfaceColor;

			result = float4( clamp( result0.x + result1.x + result2.x + result.x, 0.0f, 1.0f ),
							 clamp( result0.y + result1.y + result2.y + result.y, 0.0f, 1.0f ),
							 clamp( result0.z + result1.z + result2.z + result.z, 0.0f, 1.0f ),
							 clamp( result0.w + result1.w + result2.w + result.w, 0.0f, 1.0f ) );
		}
	}
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