// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ScreenQuadVS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

struct QuadVSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

QuadVSOutput main( uint vertId : SV_VertexID )
{
	float2 uv = float2((vertId >> 1) & 1, vertId & 1) * 2.0f;

	QuadVSOutput Output;
	Output.Position = float4(lerp(float2(-1.0f, 1.0f), float2(1.0f, -1.0f), uv), 0.0f, 1.0f);
	Output.UV = uv;
	return Output;
}