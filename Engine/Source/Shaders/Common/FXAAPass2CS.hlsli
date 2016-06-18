// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from FXAAPass2CS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "PixelPacking.hlsli"

cbuffer CB : register(b0)
{
	float2 RcpTextureSize;
};

Texture2D<float> Luma : register(t0);
StructuredBuffer<uint> WorkQueue : register(t1);
Buffer<float3> ColorQueue : register(t2);
RWTexture2D<float3> DstColor : register(u0);
RWTexture2D<uint> DstUint : register(u1); // this must alias DstColor (to load the raw uint)
SamplerState LinearSampler : register(s0);


// Note that the number of samples in each direction is one less than the number of sample distances.  The last
// is the maximum distance that should be used, but whether that sample is "good" or "bad" doesn't affect the result,
// so we don't need to load it.
#ifdef FXAA_EXTREME_QUALITY
#define NUM_SAMPLES 11
static const float s_SampleDistances[12] =	// FXAA_QUALITY__PRESET == 39
{
	1.0, 2.0, 3.0, 4.0, 5.0, 6.5, 8.5, 10.5, 12.5, 14.5, 18.5, 36.5,
};
#else
#define NUM_SAMPLES 7
static const float s_SampleDistances[8] =	// FXAA_QUALITY__PRESET == 25
{
	1.0, 2.5, 4.5, 6.5, 8.5, 10.5, 14.5, 22.5
};
#endif

[numthreads(64, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
	uint WorkHeader = WorkQueue[DTid.x];
	uint2 ST = uint2(WorkHeader >> 8, WorkHeader >> 20) & 0xFFF;
	uint GradientDir = WorkHeader & 1; // Determines which side of the pixel has the highest contrast
	float Subpix = (WorkHeader & 0xFE) / 254.0 * 0.5;      // 7-bits to encode [0, 0.5]

#ifdef VERTICAL_ORIENTATION
	float NextLuma = Luma[ST + int2(GradientDir * 2 - 1, 0)];
	float2 StartUV = (ST + float2(GradientDir, 0.5)) * RcpTextureSize;
#else
	float NextLuma = Luma[ST + int2(0, GradientDir * 2 - 1)];
	float2 StartUV = (ST + float2(0.5, GradientDir)) * RcpTextureSize;
#endif
	float ThisLuma = Luma[ST];
	float CenterLuma = (NextLuma + ThisLuma) * 0.5;         // Halfway between this and next; center of the contrasting edge
	float GradientSgn = sign(NextLuma - ThisLuma);          // Going down in brightness or up?
	float GradientMag = abs(NextLuma - ThisLuma) * 0.25;    // How much contrast?  When can we stop looking?

	float NegDist = s_SampleDistances[NUM_SAMPLES];
	float PosDist = s_SampleDistances[NUM_SAMPLES];
	bool NegGood = false;
	bool PosGood = false;

	for (uint iter = 0; iter < NUM_SAMPLES; ++iter)
	{
		const float Distance = s_SampleDistances[iter];

#ifdef VERTICAL_ORIENTATION
		float2 NegUV = StartUV - float2(0, RcpTextureSize.y) * Distance;
		float2 PosUV = StartUV + float2(0, RcpTextureSize.y) * Distance;
#else
		float2 NegUV = StartUV - float2(RcpTextureSize.x, 0) * Distance;
		float2 PosUV = StartUV + float2(RcpTextureSize.x, 0) * Distance;
#endif

		// Check for a negative endpoint
		float NegGrad = Luma.SampleLevel(LinearSampler, NegUV, 0) - CenterLuma;
		if (abs(NegGrad) >= GradientMag && Distance < NegDist)
		{
			NegDist = Distance;
			NegGood = sign(NegGrad) == GradientSgn;
		}

		// Check for a positive endpoint
		float PosGrad = Luma.SampleLevel(LinearSampler, PosUV, 0) - CenterLuma;
		if (abs(PosGrad) >= GradientMag && Distance < PosDist)
		{
			PosDist = Distance;
			PosGood = sign(PosGrad) == GradientSgn;
		}
	}

	// Ranges from 0.0 to 0.5
	float PixelShift = 0.5 - min(NegDist, PosDist) / (PosDist + NegDist);
	bool GoodSpan = NegDist < PosDist ? NegGood : PosGood;
	PixelShift = max(Subpix, GoodSpan ? PixelShift : 0.0);

	if (PixelShift > 0.01)
	{
#ifdef DEBUG_OUTPUT
		DstColor[ST] = float3(2.0 * PixelShift, 1.0 - 2.0 * PixelShift, 0);
#else
#if SUPPORT_TYPED_UAV_LOADS
		DstColor[ST] = lerp(DstColor[ST], ColorQueue[DTid.x], PixelShift);
#else
		DstColor[ST] = lerp(Unpack_R11G11B10_FLOAT(DstUint[ST]), ColorQueue[DTid.x], PixelShift);
#endif
#endif
	}
#ifdef DEBUG_OUTPUT
	else
		DstColor[ST] = float3(0, 0, 0.25);
#endif
}