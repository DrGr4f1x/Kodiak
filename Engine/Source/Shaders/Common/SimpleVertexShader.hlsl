#include "PerViewData.hlsli"
#include "StaticMeshPerObjectData.hlsli"

struct VertexShaderInput
{
	float4 pos : POSITION;
	float4 color : COLOR0;
};


// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};


// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos.xyz, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(model, pos);
	pos = mul(viewProjection, pos);
	output.pos = pos;

	// Pass the color through without modification.
	output.color = input.color.rgb;

	return output;
}