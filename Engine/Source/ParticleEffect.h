// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleEffect.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//                      James Stanard
//

#pragma once

#include "ComputeKernel.h"
#include "GpuBuffer.h"
#include "ParticleEffectProperties.h"
#include "ParticleShaderStructs.h"


namespace Kodiak
{

// Forward declarations
class ComputeCommandList;


class ParticleEffect
{
public:
	ParticleEffect(ParticleEffectProperties* effectProperties = &ParticleEffectProperties());
	void LoadDeviceResources();
	void Update(ComputeCommandList& commandList, StructuredBuffer& vertexBuffer, float timeDelta);
	float GetLifetime() { return m_effectProperties.TotalActiveLifetime; }
	float GetElapsedTime() { return m_elapsedTime; }
	void Reset();

private:
	ComputeKernel m_updateCs;
	ComputeKernel m_particleSpawnCs;
	ComputeKernel m_dispatchIndirectArgsCs;

	StructuredBuffer m_stateBuffers[2];
	uint32_t m_currentStateBuffer;
	StructuredBuffer m_randomStateBuffer;
	IndirectArgsBuffer m_dispatchIndirectArgs;
	IndirectArgsBuffer m_drawIndirectArgs;

	ParticleEffectProperties m_effectProperties;
	ParticleEffectProperties m_originalEffectProperties;
	float m_elapsedTime;
	uint32_t m_effectID;
};

} // namespace Kodiak