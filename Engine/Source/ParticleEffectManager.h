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

#include "ColorBuffer.h"
#include "ComputeKernel.h"
#include "GpuBuffer.h"
#include "ParticleEffectProperties.h"
#include "RenderThread.h"
#include "Texture.h"

namespace Kodiak
{

// Forward declarations
class Camera;
class CommandList;
class ComputeCommandList;
class DepthBuffer;
class GraphicsCommandList;
class Material;
class ParticleEffect;
class RenderPass;


enum EParticleResolution
{
	kParticleResolution_High,
	kParticleResolution_Low,
	kParticleResolution_Dynamic
};

class ParticleEffectManager
{
public:
	ParticleEffectManager();

	void Initialize(uint32_t width, uint32_t height);

	typedef uint32_t EffectHandle;
	EffectHandle PreLoadEffectResources(ParticleEffectProperties* effectProperties = &ParticleEffectProperties());
	EffectHandle InstantiateEffect(EffectHandle effectHandle);
	EffectHandle InstantiateEffect(ParticleEffectProperties* effectProperties = &ParticleEffectProperties());

	void Update(ComputeCommandList& commandList, float timeDelta);
	void Render(CommandList& commandList, std::shared_ptr<Camera> camera, ColorBuffer& colorTarget,
		DepthBuffer& depthBuffer, ColorBuffer& linearDepth);

	// Feature toggles
	ThreadParameter<bool> Enable;
	ThreadParameter<bool> PauseSim;
	ThreadParameter<bool> EnableTiledRendering;
	ThreadParameter<bool> EnableSpriteSort;
	ThreadParameter<EParticleResolution> ParticleResolution;
	ThreadParameter<float> DynamicResLevel;
	ThreadParameter<float> MipBias;

private:
	void MaintainTextureList(ParticleEffectProperties* effectProperties);
	void SetFinalBuffers(ComputeCommandList& commandList);

	struct CBChangesPerView
	{
		Math::Matrix4 InvView;
		Math::Matrix4 ViewProj;

		float VertCotangent;
		float AspectRatio;
		float RcpFarZ;
		float InvertZ;

		float BufferWidth;
		float BufferHeight;
		float RcpBufferWidth;
		float RcpBufferHeight;

		uint32_t BinsPerRow;
		uint32_t TileRowPitch;
		uint32_t TilesPerRow;
		uint32_t TilesPerCol;
	};

	void RenderTiles(ComputeCommandList& commandList, const CBChangesPerView& cbData, ColorBuffer& colorTarget,
		DepthBuffer& depthTarget, ColorBuffer& linearDepth);

	void RenderSprites(GraphicsCommandList& commandList, const CBChangesPerView& cbData, ColorBuffer& colorTarget,
		DepthBuffer& depthTarget, ColorBuffer& linearDepth);

	void BitonicSort(ComputeCommandList& commandList);

private:
	// Compute kernels
	ComputeKernel	m_particleFinalDispatchIndirectArgsCs;
	ComputeKernel	m_particleLargeBinCullingCs;
	ComputeKernel	m_particleBinCullingCs;
	ComputeKernel	m_particleTileCullingCs;
	ComputeKernel	m_particleTileRenderSlowCs[3];
	ComputeKernel	m_particleTileRenderFastCs[3];
	ComputeKernel	m_particleDepthBoundsCs;
	ComputeKernel	m_particleSortIndirectArgsCs;
	ComputeKernel	m_particlePreSortCs;
	ComputeKernel	m_particleInnerSortCs[7];
	ComputeKernel	m_particleOuterSortCs[28];

	// Materials
	std::shared_ptr<Material>		m_particleRenderMaterial;
	std::shared_ptr<RenderPass>		m_particleRenderPass;

	// Internal render targets and UAV buffers
	IndirectArgsBuffer	m_drawIndirectArgs;
	IndirectArgsBuffer	m_finalDispatchIndirectArgs;
	StructuredBuffer	m_spriteVertexBuffer;
	StructuredBuffer	m_visibleParticleBuffer;
	StructuredBuffer	m_spriteIndexBuffer;
	IndirectArgsBuffer	m_sortIndirectArgs;
	IndirectArgsBuffer	m_tileDrawDispatchIndirectArgs;
	StructuredBuffer	m_binParticles[2];
	StructuredBuffer	m_binCounters[2];
	StructuredBuffer	m_tileCounters;
	ByteAddressBuffer	m_tileHitMasks;
	StructuredBuffer	m_tileDrawPackets;
	StructuredBuffer	m_tileFastDrawPackets;
	Texture				m_textureArray;
	ColorBuffer			m_minMaxDepth8;
	ColorBuffer			m_minMaxDepth16;
	ColorBuffer			m_minMaxDepth32;

	// Feature toggles
	bool m_enable{ true };
	bool m_pauseSim{ false };
	bool m_enableTiledRendering{ true };
	bool m_enableSpriteSort{ true };

	// Parameters
	EParticleResolution m_tiledRes{ kParticleResolution_High };
	float m_dynamicResLevel{ 0.0f };
	float m_mipBias{ 0.0f };

	// State
	bool		m_initialized{ false };
	uint32_t	m_reproFrame{ 0 };
	uint32_t	m_totalElapsedFrames{ 0 };
	std::vector<std::unique_ptr<ParticleEffect>>	m_particleEffectsPool;
	std::vector<ParticleEffect*>					m_particleEffectsActive;
	std::vector<std::string>						m_textureNameArray;
};

} // namespace Kodiak