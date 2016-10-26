// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleEffectManager.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//                      James Stanard
//

#pragma once

#include "ParticleEffectProperties.h"
#include "RenderThread.h"

namespace Kodiak
{

// Forward declarations
class ByteAddressBuffer;
class ComputeKernel;
class IndirectArgsBuffer;
class ParticleEffect;
class StructuredBuffer;
class Texture;


class ParticleEffectManager
{
public:
	ParticleEffectManager();

	void Initialize(uint32_t width, uint32_t height);

	typedef uint32_t EffectHandle;
	EffectHandle PreLoadEffectResources(ParticleEffectProperties* effectProperties = &ParticleEffectProperties());
	EffectHandle InstantiateEffect(EffectHandle effectHandle);
	EffectHandle InstantiateEffect(ParticleEffectProperties* effectProperties = &ParticleEffectProperties());

	// Feature toggles
	ThreadParameter<bool> Enable;
	ThreadParameter<bool> PauseSim;
	ThreadParameter<bool> EnableTiledRendering;

private:
	void MaintainTextureList(ParticleEffectProperties* effectProperties);

private:
	// Compute kernels
	std::shared_ptr<ComputeKernel>	m_particleFinalDispatchIndirectArgsCs;
	std::shared_ptr<ComputeKernel>	m_particleLargeBinCullingCs;
	std::shared_ptr<ComputeKernel>	m_particleBinCullingCs;
	std::shared_ptr<ComputeKernel>	m_particleTileCullingCs;
	std::shared_ptr<ComputeKernel>	m_particleTileRenderSlowCs[3];
	std::shared_ptr<ComputeKernel>	m_particleTileRenderFastCs[3];
	std::shared_ptr<ComputeKernel>	m_particleDepthBoundsCs;
	std::shared_ptr<ComputeKernel>	m_particleSortIndirectArgsCs;
	std::shared_ptr<ComputeKernel>	m_particlePreSortCs;
	std::shared_ptr<ComputeKernel>	m_particleInnerSortCs;
	std::shared_ptr<ComputeKernel>	m_particleOuterSortCs;

	// Internal render targets and UAV buffers
	std::shared_ptr<IndirectArgsBuffer> m_drawIndirectArgs;
	std::shared_ptr<IndirectArgsBuffer> m_finalDispatchIndirectArgs;
	std::shared_ptr<StructuredBuffer>	m_spriteVertexBuffer;
	std::shared_ptr<StructuredBuffer>	m_visibleParticleBuffer;
	std::shared_ptr<StructuredBuffer>	m_spriteIndexBuffer;
	std::shared_ptr<IndirectArgsBuffer>	m_sortIndirectArgs;
	std::shared_ptr<IndirectArgsBuffer>	m_tileDrawDispatchIndirectArgs;
	std::shared_ptr<StructuredBuffer>	m_binParticles[2];
	std::shared_ptr<StructuredBuffer>	m_binCounters[2];
	std::shared_ptr<StructuredBuffer>	m_tileCounters;
	std::shared_ptr<ByteAddressBuffer>	m_tileHitMasks;
	std::shared_ptr<StructuredBuffer>	m_tileDrawPackets;
	std::shared_ptr<StructuredBuffer>	m_tileFastDrawPackets;
	std::shared_ptr<Texture>			m_textureArray;

	// Feature toggles
	bool m_enable{ true };
	bool m_pauseSim{ false };
	bool m_enableTiledRendering{ true };

	// State
	bool		m_initialized{ false };
	uint32_t	m_reproFrame{ 0 };
	uint32_t	m_totalElapsedFrames{ 0 };
	std::vector<std::unique_ptr<ParticleEffect>>	m_particleEffectsPool;
	std::vector<ParticleEffect*>					m_particleEffectsActive;
	std::vector<std::string>						m_textureNameArray;
};

} // namespace Kodiak