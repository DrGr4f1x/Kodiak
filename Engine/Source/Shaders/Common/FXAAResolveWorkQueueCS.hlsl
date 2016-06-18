// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from FXAAResolveWorkQueueCS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#if DX12
ByteAddressBuffer WorkCounterH : register(t0);
ByteAddressBuffer WorkCounterV : register(t1);
#endif
RWByteAddressBuffer IndirectParams : register(u0);
RWStructuredBuffer<uint> WorkQueueH : register(u1);
RWStructuredBuffer<uint> WorkQueueV : register(u2);

#if DX11
groupshared uint PixelCountH;
groupshared uint PixelCountV;
#endif

[numthreads(64, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
#if DX12
	uint PixelCountH = WorkCounterH.Load(0);
	uint PixelCountV = WorkCounterV.Load(0);
#endif

#if DX11
	if (GI == 0)
	{
		PixelCountH = WorkQueueH.IncrementCounter();
		PixelCountV = WorkQueueV.IncrementCounter();
	}
	GroupMemoryBarrierWithGroupSync();
#endif

	uint PaddedCountH = (PixelCountH + 63) & ~63;
	uint PaddedCountV = (PixelCountV + 63) & ~63;

	// Write out padding to the buffer
	if (GI + PixelCountH < PaddedCountH)
		WorkQueueH[PixelCountH + GI] = 0xffffffff;

	// Write out padding to the buffer
	if (GI + PixelCountV < PaddedCountV)
		WorkQueueV[PixelCountV + GI] = 0xffffffff;

	if (GI == 0)
	{
		IndirectParams.Store(0, PaddedCountH >> 6);
		IndirectParams.Store(12, PaddedCountV >> 6);
	}
}