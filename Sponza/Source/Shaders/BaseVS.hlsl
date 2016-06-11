#include "..\..\..\Engine\Source\Shaders\Common\PerViewData.hlsli"
#include "..\..\..\Engine\Source\Shaders\Common\StaticMeshPerObjectData.hlsli"

struct VertexShaderInput
{
	float3 position : POSITION;
	float2 texcoord0 : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};


struct VertexShaderOutput
{
	float4 position : SV_Position;
	float2 texcoord0 : texcoord0;
	float3 viewDir : texcoord1;
	float3 shadowCoord : texcoord2;
	float3 normal : normal;
	float3 tangent : tangent;
	float3 bitangent : bitangent;
};


VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 pos = float4(input.position, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(model, pos);
	pos = mul(viewProjection, pos);
	output.position = pos;

	output.texcoord0 = input.texcoord0;
	output.viewDir = input.position - viewPosition;
	output.shadowCoord = mul(modelToShadow, float4(input.position, 1.0)).xyz;
	output.normal = input.normal;
	output.tangent = input.tangent;
	output.bitangent = input.bitangent;

	return output;
}