// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ExtractLumaCS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "ShaderUtility.hlsli"

SamplerState LinearSampler : register(s0);
Texture2D<float3> SourceTex : register(t0);
RWTexture2D<uint> LumaResult : register(u0);

cbuffer cb0
{
	float2 g_inverseOutputSize;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// We need the scale factor and the size of one pixel so that our four samples are right in the middle
	// of the quadrant they are covering.
	float2 uv = DTid.xy * g_inverseOutputSize;
	float2 offset = g_inverseOutputSize * 0.25f;

	// Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
	float3 color1 = SourceTex.SampleLevel(LinearSampler, uv + float2(-offset.x, -offset.y), 0);
	float3 color2 = SourceTex.SampleLevel(LinearSampler, uv + float2(offset.x, -offset.y), 0);
	float3 color3 = SourceTex.SampleLevel(LinearSampler, uv + float2(-offset.x, offset.y), 0);
	float3 color4 = SourceTex.SampleLevel(LinearSampler, uv + float2(offset.x, offset.y), 0);

	// Compute average luminance
	float luma = RGBToLuminance(color1 + color2 + color3 + color4) * 0.25;

	// Logarithmically compress the range [2^-12, 2^4) to [0, 255]
	float logLuma = log2(max(1.0 / 4096.0, luma)) + 12.0;
	uint quantizedLog = min(255, (uint)round(logLuma * 16.0));

	// Store result
	LumaResult[DTid.xy] = quantizedLog;
}