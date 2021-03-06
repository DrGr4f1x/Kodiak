// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleFinalDispatchIndirectArgsCS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//

#include "ParticleUtility.hlsli"

ByteAddressBuffer g_FinalInstanceCounter : register(t0);
RWByteAddressBuffer g_NumThreadGroups : register(u0);
RWByteAddressBuffer g_DrawIndirectArgs : register (u1);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint particleCount = g_FinalInstanceCounter.Load(0);
	g_NumThreadGroups.Store3(0, uint3((particleCount + 63) / 64, 1, 1));
	g_DrawIndirectArgs.Store(4, particleCount);
}