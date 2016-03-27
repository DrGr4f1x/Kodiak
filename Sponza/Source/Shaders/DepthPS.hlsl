struct PixelShaderInput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
};

Texture2D<float4> texDiffuse : register(t0);
SamplerState AnisotropicWrap : register(s0);

void main(PixelShaderInput input)
{
	if (texDiffuse.Sample(AnisotropicWrap, input.texcoord).a < 0.5f)
	{
		discard;
	}
}