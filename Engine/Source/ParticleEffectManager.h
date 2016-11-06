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
class Camera;
class ColorBuffer;
class CommandList;
class ComputeCommandList;
class ComputeKernel;
class DepthBuffer;
class GraphicsCommandList;
class IndirectArgsBuffer;
class Material;
class ParticleEffect;
class RenderPass;
class StructuredBuffer;
class Texture;


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
	void Render(CommandList& commandList, std::shared_ptr<Camera> camera, std::shared_ptr<ColorBuffer> colorTarget,
		std::shared_ptr<DepthBuffer> depthBuffer, std::shared_ptr<ColorBuffer> linearDepth);

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

	void RenderTiles(ComputeCommandList& commandList, const CBChangesPerView& cbData, std::shared_ptr<ColorBuffer> colorTarget,
		std::shared_ptr<DepthBuffer> depthTarget, std::shared_ptr<ColorBuffer> linearDepth);

	void RenderSprites(GraphicsCommandList& commandList, const CBChangesPerView& cbData, std::shared_ptr<ColorBuffer> colorTarget,
		std::shared_ptr<DepthBuffer> depthTarget, std::shared_ptr<ColorBuffer> linearDepth);

	void BitonicSort(ComputeCommandList& commandList);

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
	std::shared_ptr<ComputeKernel>	m_particleInnerSortCs[7];
	std::shared_ptr<ComputeKernel>	m_particleOuterSortCs[28];

	// Materials
	std::shared_ptr<Material>		m_particleRenderMaterial;
	std::shared_ptr<RenderPass>		m_particleRenderPass;

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
	std::shared_ptr<ColorBuffer>		m_minMaxDepth8;
	std::shared_ptr<ColorBuffer>		m_minMaxDepth16;
	std::shared_ptr<ColorBuffer>		m_minMaxDepth32;

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