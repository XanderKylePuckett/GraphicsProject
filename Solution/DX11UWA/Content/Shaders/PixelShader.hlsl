texture2D tex : register( t0 );
SamplerState samplerstate : register( s0 );

cbuffer LightingConstantBuffer : register( b0 )
{
	float4 lightState;
	float4 dLightDirection;
	float4 pLightPos[ 3 ];
	float4 pLightColorRadius[ 3 ];
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 posW : PW;
	float4 texcoord : TEXCOORD;
	float4 normal : NORMAL;
};

float4 main( PixelShaderInput input ) : SV_TARGET
{
	float4 surfaceColor = tex.Sample( samplerstate, input.texcoord.xy );
	float4 result;
	if ( lightState.x <= 0.5f && lightState.y <= 0.5f )
		return surfaceColor;
	else
	{
		result = float4( 0.0f, 0.0f, 0.0f, 1.0f );
		if ( lightState.x > 0.5f )
		{
			float lightRatio = clamp( dot( -normalize( dLightDirection.xyz ), normalize( input.normal.xyz ) ), 0.0f, 1.0f );
			float4 lightColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
			result = lightRatio * lightColor * surfaceColor;
		}
		if ( lightState.y > 0.5f )
		{
			float3 lightDirs[ 3 ];
			float4 lightColors[ 3 ];
			float lightRatios[ 3 ];
			float4 results[ 3 ];

			for ( int i = 0; i < 3; ++i )
			{
				lightDirs[ i ] = normalize( pLightPos[ i ].xyz - input.posW.xyz );
				lightColors[ i ] = float4( pLightColorRadius[ i ].xyz, 1.0f );
				lightRatios[ i ] = clamp( dot( lightDirs[ i ], normalize( input.normal.xyz ) ), 0.0f, 1.0f );
				results[ i ] = lightRatios[ i ] * lightColors[ i ] * surfaceColor;
			}

			return float4( clamp( results[ 0 ].x + results[ 1 ].x + results[ 2 ].x + result.x, 0.0f, 1.0f ),
							 clamp( results[ 0 ].y + results[ 1 ].y + results[ 2 ].y + result.y, 0.0f, 1.0f ),
							 clamp( results[ 0 ].z + results[ 1 ].z + results[ 2 ].z + result.z, 0.0f, 1.0f ),
							 clamp( results[ 0 ].w + results[ 1 ].w + results[ 2 ].w + result.w, 0.0f, 1.0f ) );
		}
		return result;
	}
	return float4( 0.0f, 0.0f, 0.0f, 1.0f );
}