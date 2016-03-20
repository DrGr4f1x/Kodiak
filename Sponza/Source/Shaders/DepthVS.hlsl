#include "..\..\..\Engine\Source\Shaders\Common\PerViewData.hlsli"
#include "..\..\..\Engine\Source\Shaders\Common\StaticMeshPerObjectData.hlsli"


struct VertexShaderInput
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};


struct VertexShaderOutput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
};


VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4 pos = float4(input.position, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.position = pos;

	output.texcoord = input.texcoord;

	return output;
}