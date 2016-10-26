// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleEffectManager.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//                      James Stanard
//

#include "Stdafx.h"

#include "ParticleEffectManager.h"

#include "CommandList.h"
#include "ComputeKernel.h"
#include "ComputeParameter.h"
#include "ComputeResource.h"
#include "DeviceManager.h"
#include "Format.h"
#include "GpuBuffer.h"
#include "ParticleEffect.h"
#include "ParticleEffectProperties.h"
#include "Random.h"
#include "Texture.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


#define EFFECTS_ERROR uint32_t(0xFFFFFFFF)

#define MAX_TOTAL_PARTICLES 0x40000		// 256k (18-bit indices)
#define MAX_PARTICLES_PER_BIN 1024
#define BIN_SIZE_X 128
#define BIN_SIZE_Y 64
#define TILE_SIZE 16

// It's good to have 32 tiles per bin to maximize the tile culling phase
#define TILES_PER_BIN_X (BIN_SIZE_X / TILE_SIZE)
#define TILES_PER_BIN_Y (BIN_SIZE_Y / TILE_SIZE)
#define TILES_PER_BIN (TILES_PER_BIN_X * TILES_PER_BIN_Y)


ParticleEffectManager::ParticleEffectManager()
	: Enable(m_enable)
	, PauseSim(m_pauseSim)
	, EnableTiledRendering(m_enableTiledRendering)
{}


void ParticleEffectManager::Initialize(uint32_t width, uint32_t height)
{
	m_particleFinalDispatchIndirectArgsCs = make_shared<ComputeKernel>();
	m_particleFinalDispatchIndirectArgsCs->SetComputeShaderPath("Engine\\ParticleFinalDispatchIndirectArgsCS");
	auto waitTask = m_particleFinalDispatchIndirectArgsCs->loadTask;

	m_particleLargeBinCullingCs = make_shared<ComputeKernel>();
	m_particleLargeBinCullingCs->SetComputeShaderPath("Engine\\ParticleLargeBinCullingCS");
	waitTask = waitTask && m_particleLargeBinCullingCs->loadTask;

	m_particleBinCullingCs = make_shared<ComputeKernel>();
	m_particleBinCullingCs->SetComputeShaderPath("Engine\\ParticleBinCullingCS");
	waitTask = waitTask && m_particleBinCullingCs->loadTask;

	m_particleTileCullingCs = make_shared<ComputeKernel>();
	m_particleTileCullingCs->SetComputeShaderPath("Engine\\ParticleTileCullingCS");
	waitTask = waitTask && m_particleTileCullingCs->loadTask;

	if (DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{
		m_particleTileRenderSlowCs[0] = make_shared<ComputeKernel>();
		m_particleTileRenderSlowCs[0]->SetComputeShaderPath("Engine\\ParticleTileRender2CS");
		waitTask = waitTask && m_particleTileRenderSlowCs[0]->loadTask;

		m_particleTileRenderSlowCs[1] = make_shared<ComputeKernel>();
		m_particleTileRenderSlowCs[1]->SetComputeShaderPath("Engine\\ParticleTileRenderSlowLowRes2CS");
		waitTask = waitTask && m_particleTileRenderSlowCs[1]->loadTask;

		m_particleTileRenderSlowCs[2] = make_shared<ComputeKernel>();
		m_particleTileRenderSlowCs[2]->SetComputeShaderPath("Engine\\ParticleTileRenderSlowDynamic2CS");
		waitTask = waitTask && m_particleTileRenderSlowCs[2]->loadTask;

		m_particleTileRenderFastCs[0] = make_shared<ComputeKernel>();
		m_particleTileRenderFastCs[0]->SetComputeShaderPath("Engine\\ParticleTileRenderFast2CS");
		waitTask = waitTask && m_particleTileRenderFastCs[0]->loadTask;

		m_particleTileRenderFastCs[1] = make_shared<ComputeKernel>();
		m_particleTileRenderFastCs[1]->SetComputeShaderPath("Engine\\ParticleTileRenderFastLowRes2CS");
		waitTask = waitTask && m_particleTileRenderFastCs[1]->loadTask;

		m_particleTileRenderFastCs[2] = make_shared<ComputeKernel>();
		m_particleTileRenderFastCs[2]->SetComputeShaderPath("Engine\\ParticleTileRenderFastDynamic2CS");
		waitTask = waitTask && m_particleTileRenderFastCs[2]->loadTask;
	}
	else
	{
		m_particleTileRenderSlowCs[0] = make_shared<ComputeKernel>();
		m_particleTileRenderSlowCs[0]->SetComputeShaderPath("Engine\\ParticleTileRenderCS");
		waitTask = waitTask && m_particleTileRenderSlowCs[0]->loadTask;

		m_particleTileRenderSlowCs[1] = make_shared<ComputeKernel>();
		m_particleTileRenderSlowCs[1]->SetComputeShaderPath("Engine\\ParticleTileRenderSlowLowResCS");
		waitTask = waitTask && m_particleTileRenderSlowCs[1]->loadTask;

		m_particleTileRenderSlowCs[2] = make_shared<ComputeKernel>();
		m_particleTileRenderSlowCs[2]->SetComputeShaderPath("Engine\\ParticleTileRenderSlowDynamicCS");
		waitTask = waitTask && m_particleTileRenderSlowCs[2]->loadTask;

		m_particleTileRenderFastCs[0] = make_shared<ComputeKernel>();
		m_particleTileRenderFastCs[0]->SetComputeShaderPath("Engine\\ParticleTileRenderFastCS");
		waitTask = waitTask && m_particleTileRenderFastCs[0]->loadTask;

		m_particleTileRenderFastCs[1] = make_shared<ComputeKernel>();
		m_particleTileRenderFastCs[1]->SetComputeShaderPath("Engine\\ParticleTileRenderFastLowResCS");
		waitTask = waitTask && m_particleTileRenderFastCs[1]->loadTask;

		m_particleTileRenderFastCs[2] = make_shared<ComputeKernel>();
		m_particleTileRenderFastCs[2]->SetComputeShaderPath("Engine\\ParticleTileRenderFastDynamicCS");
		waitTask = waitTask && m_particleTileRenderFastCs[2]->loadTask;
	}

	m_particleDepthBoundsCs = make_shared<ComputeKernel>();
	m_particleDepthBoundsCs->SetComputeShaderPath("Engine\\ParticleDepthBoundsCS");
	waitTask = waitTask && m_particleDepthBoundsCs->loadTask;

	m_particleSortIndirectArgsCs = make_shared<ComputeKernel>();
	m_particleSortIndirectArgsCs->SetComputeShaderPath("Engine\\ParticleSortIndirectArgsCS");
	waitTask = waitTask && m_particleSortIndirectArgsCs->loadTask;

	m_particlePreSortCs = make_shared<ComputeKernel>();
	m_particlePreSortCs->SetComputeShaderPath("Engine\\ParticlePreSortCS");
	waitTask = waitTask && m_particlePreSortCs->loadTask;

	m_particleInnerSortCs = make_shared<ComputeKernel>();
	m_particleInnerSortCs->SetComputeShaderPath("Engine\\ParticleInnerSortCS");
	waitTask = waitTask && m_particleInnerSortCs->loadTask;

	m_particleOuterSortCs = make_shared<ComputeKernel>();
	m_particleOuterSortCs->SetComputeShaderPath("Engine\\ParticleOuterSortCS");
	waitTask = waitTask && m_particleOuterSortCs->loadTask;

	m_drawIndirectArgs = make_shared<IndirectArgsBuffer>();
	__declspec(align(16)) UINT initialDrawIndirectArgs[4] = { 4, 0, 0, 0 };
	m_drawIndirectArgs->Create("ParticleEffects::DrawIndirectArgs", 1, 4 * sizeof(UINT), initialDrawIndirectArgs);

	m_finalDispatchIndirectArgs = make_shared<IndirectArgsBuffer>();
	__declspec(align(16)) UINT initialDispatchIndirectArgs[6] = { 0, 1, 1, 0, 1, 1 };
	m_finalDispatchIndirectArgs->Create("ParticleEffects::FinalDispatchIndirectArgs", 1, 3 * sizeof(UINT), initialDispatchIndirectArgs);

	m_spriteVertexBuffer = make_shared<StructuredBuffer>();
	m_spriteVertexBuffer->Create("ParticleEffects::SpriteVertexBuffer", MAX_TOTAL_PARTICLES, sizeof(ParticleVertex));

	m_visibleParticleBuffer = make_shared<StructuredBuffer>();
	m_visibleParticleBuffer->Create("ParticleEffects::VisibleParticleBuffer", MAX_TOTAL_PARTICLES, sizeof(ParticleScreenData));

	m_spriteIndexBuffer = make_shared<StructuredBuffer>();
	m_spriteIndexBuffer->Create("ParticleEffects::SpriteIndexBuffer", MAX_TOTAL_PARTICLES, sizeof(UINT));

	m_sortIndirectArgs = make_shared<IndirectArgsBuffer>();
	m_sortIndirectArgs->Create("ParticleEffects::SortIndirectArgs", 8, 3 * sizeof(UINT));

	m_tileDrawDispatchIndirectArgs = make_shared<IndirectArgsBuffer>();
	m_tileDrawDispatchIndirectArgs->Create("ParticleEffects::DrawPackets_IArgs", 2, 3 * sizeof(UINT), initialDispatchIndirectArgs);

	const uint32_t largeBinsPerRow = DivideByMultiple(width, 4 * BIN_SIZE_X);
	const uint32_t largeBinsPerCol = DivideByMultiple(height, 4 * BIN_SIZE_Y);
	const uint32_t binsPerRow = largeBinsPerRow * 4;
	const uint32_t binsPerCol = largeBinsPerCol * 4;
	const uint32_t maxParticlesPerLargeBin = MAX_PARTICLES_PER_BIN * 16;
	const uint32_t particleBinCapacity = largeBinsPerRow * largeBinsPerCol * maxParticlesPerLargeBin;
	const uint32_t tilesPerRow = DivideByMultiple(width, TILE_SIZE);
	const uint32_t tilesPerCol = DivideByMultiple(height, TILE_SIZE);

	// Padding is necessary to eliminate bounds checking when outputting data to bins or tiles.
	const uint32_t paddedTilesPerRow = AlignUp(tilesPerRow, TILES_PER_BIN_X * 4);
	const uint32_t paddedTilesPerCol = AlignUp(tilesPerCol, TILES_PER_BIN_Y * 4);

	m_binParticles[0] = make_shared<StructuredBuffer>();
	m_binParticles[1] = make_shared<StructuredBuffer>();
	m_binParticles[0]->Create("ParticleEffects::BinParticles[0]", particleBinCapacity, sizeof(UINT));
	m_binParticles[1]->Create("ParticleEffects::BinParticles[0]", particleBinCapacity, sizeof(UINT));

	m_binCounters[0] = make_shared<StructuredBuffer>();
	m_binCounters[1] = make_shared<StructuredBuffer>();
	m_binCounters[0]->Create("ParticleEffects::LargeBinCounters", largeBinsPerRow * largeBinsPerCol, sizeof(UINT));
	m_binCounters[1]->Create("ParticleEffects::BinCounters", binsPerRow * binsPerCol, sizeof(UINT));

	m_tileCounters = make_shared<StructuredBuffer>();
	m_tileCounters->Create("ParticleEffects::TileCounters", paddedTilesPerRow * paddedTilesPerCol, sizeof(UINT));

	m_tileHitMasks = make_shared<ByteAddressBuffer>();
	m_tileHitMasks->Create("ParticleEffects::TileHitMasks", paddedTilesPerRow * paddedTilesPerCol, MAX_PARTICLES_PER_BIN / 8);

	m_tileDrawPackets = make_shared<StructuredBuffer>();
	m_tileDrawPackets->Create("ParticleEffects::DrawPackets", tilesPerRow * tilesPerCol, sizeof(UINT));

	m_tileFastDrawPackets = make_shared<StructuredBuffer>();
	m_tileFastDrawPackets->Create("ParticleEffects::FastDrawPackets", tilesPerRow * tilesPerCol, sizeof(UINT));

	m_textureArray = make_shared<Texture>();
	m_textureArray->CreateArray(64, 64, 16, 4, ColorFormat::BC3_UNorm_sRGB);

	if (m_reproFrame > 0)
	{
		g_rng.SetSeed(1);
	}

	waitTask.wait();
	m_initialized = true;
}


// Returns index into Pool
ParticleEffectManager::EffectHandle ParticleEffectManager::PreLoadEffectResources(ParticleEffectProperties* effectProperties)
{
	if (!m_initialized)
	{
		return EFFECTS_ERROR;
	}

	static std::mutex textureMutex;
	textureMutex.lock();

	MaintainTextureList(effectProperties);
	m_particleEffectsPool.emplace_back(new ParticleEffect(effectProperties));

	EffectHandle index = (EffectHandle)m_particleEffectsPool.size() - 1;

	textureMutex.unlock();

	m_particleEffectsPool[index]->LoadDeviceResources();

	return index;
}


// Returns index into Active
ParticleEffectManager::EffectHandle ParticleEffectManager::InstantiateEffect(ParticleEffectManager::EffectHandle effectHandle)
{
	if (!m_initialized || effectHandle >= m_particleEffectsPool.size() || effectHandle < 0)
	{
		return EFFECTS_ERROR;
	}

	ParticleEffect* effect = m_particleEffectsPool[effectHandle].get();
	if (effect != nullptr)
	{
		static std::mutex instantiateEffectFromPoolMutex;
		instantiateEffectFromPoolMutex.lock();
		m_particleEffectsActive.push_back(effect);
		instantiateEffectFromPoolMutex.unlock();
	}
	else
	{
		return EFFECTS_ERROR;
	}

	EffectHandle index = (EffectHandle)m_particleEffectsActive.size() - 1;
	return index;

}


// Returns index into Active
ParticleEffectManager::EffectHandle ParticleEffectManager::InstantiateEffect(ParticleEffectProperties* effectProperties)
{
	if (!m_initialized)
	{
		return EFFECTS_ERROR;
	}

	static std::mutex instantiateNewEffectMutex;
	instantiateNewEffectMutex.lock();

	MaintainTextureList(effectProperties);
	ParticleEffect* newEffect = new ParticleEffect(effectProperties);
	m_particleEffectsPool.emplace_back(newEffect);
	m_particleEffectsActive.push_back(newEffect);

	instantiateNewEffectMutex.unlock();

	EffectHandle index = (EffectHandle)m_particleEffectsActive.size() - 1;
	m_particleEffectsActive[index]->LoadDeviceResources();
	return index;
}


void ParticleEffectManager::MaintainTextureList(ParticleEffectProperties* effectProperties)
{
	std::string name = effectProperties->TexturePath;

	for (uint32_t i = 0; i < m_textureNameArray.size(); i++)
	{
		if (name.compare(m_textureNameArray[i]) == 0)
		{
			effectProperties->EmitProperties.TextureID = i;
			return;
		}
	}

	m_textureNameArray.push_back(name);
	UINT textureID = (UINT)(m_textureNameArray.size() - 1);
	effectProperties->EmitProperties.TextureID = textureID;

	auto texture = Texture::Load(name, true);
	texture->loadTask.wait();

	GpuResource& particleTexture = *texture;
	CommandList::InitializeTextureArraySlice(*m_textureArray, textureID, particleTexture);
}