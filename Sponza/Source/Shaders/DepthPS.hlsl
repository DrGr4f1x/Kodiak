struct PixelShaderInput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
};

Texture2D<float4> texDiffuse : register(t0);
SamplerState sampler0 : register(s0);

void main(PixelShaderInput input)
{
	if (texDiffuse.Sample(sampler0, input.texcoord).a < 0.5f)
	{
		discard;
	}
}