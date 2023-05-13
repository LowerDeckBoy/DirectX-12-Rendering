

Texture2D baseColorTexture		: register(t0);
Texture2D normalTexture			: register(t1);
Texture2D metalRoughnessTexture : register(t2);
Texture2D emissiveTexture		: register(t3);

SamplerState textureSampler		: register(s0);

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}