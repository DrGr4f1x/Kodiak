// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from GenerateHistogramCS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

Texture2D<uint> LumaBuf : register(t0);
RWByteAddressBuffer Histogram : register(u0);

groupshared uint g_TileHistogram[256];

[numthreads(16, 16, 1)]
void main(uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID)
{
	g_TileHistogram[GI] = 0;

	GroupMemoryBarrierWithGroupSync();

	// Loop 24 times until the entire column has been processed
	for (uint TopY = 0; TopY < 384; TopY += 16)
	{
		uint QuantizedLogLuma = LumaBuf[DTid.xy + uint2(0, TopY)];
		InterlockedAdd(g_TileHistogram[QuantizedLogLuma], 1);
	}

	GroupMemoryBarrierWithGroupSync();

	Histogram.InterlockedAdd(GI * 4, g_TileHistogram[GI]);
}