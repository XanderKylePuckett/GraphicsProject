// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 texcoord : TEXCOORD;
	float4 normal : NORMAL;
};

// A pass-through function for the (interpolated) color data.
float4 main( PixelShaderInput input ) : SV_TARGET
{
	return float4( input.texcoord );
}
