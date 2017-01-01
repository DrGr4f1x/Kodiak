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

#include "Camera.h"
#include "CommandList.h"
#include "CommonStates.h"
#include "ComputeConstantBuffer.h"
#include "ComputeParameter.h"
#include "ComputeResource.h"
#include "DepthBuffer.h"
#include "DeviceManager.h"
#include "Effect.h"
#include "Format.h"
#include "Material.h"
#include "MaterialConstantBuffer.h"
#include "MaterialResource.h"
#include "ParticleEffect.h"
#include "ParticleEffectProperties.h"
#include "Paths.h"
#include "Random.h"
#include "Rectangle.h"
#include "RenderPass.h"
#include "Texture.h"
#include "Viewport.h"


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


#if 0
ParticleEffectManager::ParticleEffectManager()
	: Enable(m_enable)
	, PauseSim(m_pauseSim)
	, EnableTiledRendering(m_enableTiledRendering)
	, EnableSpriteSort(m_enableSpriteSort)
	, ParticleResolution(m_tiledRes)
	, DynamicResLevel(m_dynamicResLevel)
	, MipBias(m_mipBias)
{}


void ParticleEffectManager::Initialize(uint32_t width, uint32_t height)
{
	auto waitTask = concurrency::create_task([] {});

	m_particleFinalDispatchIndirectArgsCs.SetComputeShaderPath("Engine\\ParticleFinalDispatchIndirectArgsCS", waitTask);
	m_particleLargeBinCullingCs.SetComputeShaderPath("Engine\\ParticleLargeBinCullingCS", waitTask);
	m_particleBinCullingCs.SetComputeShaderPath("Engine\\ParticleBinCullingCS", waitTask);
	m_particleTileCullingCs.SetComputeShaderPath("Engine\\ParticleTileCullingCS", waitTask);
	
	if (DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{
		m_particleTileRenderSlowCs[0].SetComputeShaderPath("Engine\\ParticleTileRender2CS", waitTask);
		m_particleTileRenderSlowCs[1].SetComputeShaderPath("Engine\\ParticleTileRenderSlowLowRes2CS", waitTask);
		m_particleTileRenderSlowCs[2].SetComputeShaderPath("Engine\\ParticleTileRenderSlowDynamic2CS", waitTask);
		
		m_particleTileRenderFastCs[0].SetComputeShaderPath("Engine\\ParticleTileRenderFast2CS", waitTask);
		m_particleTileRenderFastCs[1].SetComputeShaderPath("Engine\\ParticleTileRenderFastLowRes2CS", waitTask);
		m_particleTileRenderFastCs[2].SetComputeShaderPath("Engine\\ParticleTileRenderFastDynamic2CS", waitTask);
	}
	else
	{
		m_particleTileRenderSlowCs[0].SetComputeShaderPath("Engine\\ParticleTileRenderCS", waitTask);
		m_particleTileRenderSlowCs[1].SetComputeShaderPath("Engine\\ParticleTileRenderSlowLowResCS", waitTask);
		m_particleTileRenderSlowCs[2].SetComputeShaderPath("Engine\\ParticleTileRenderSlowDynamicCS", waitTask);

		m_particleTileRenderFastCs[0].SetComputeShaderPath("Engine\\ParticleTileRenderFastCS", waitTask);
		m_particleTileRenderFastCs[1].SetComputeShaderPath("Engine\\ParticleTileRenderFastLowResCS", waitTask);
		m_particleTileRenderFastCs[2].SetComputeShaderPath("Engine\\ParticleTileRenderFastDynamicCS", waitTask);
	}

	m_particleDepthBoundsCs.SetComputeShaderPath("Engine\\ParticleDepthBoundsCS", waitTask);
	m_particleSortIndirectArgsCs.SetComputeShaderPath("Engine\\ParticleSortIndirectArgsCS", waitTask);
	m_particlePreSortCs.SetComputeShaderPath("Engine\\ParticlePreSortCS", waitTask);
	
	auto particleRenderEffect = make_shared<Effect>();
	particleRenderEffect->SetName("Particle effect");
	particleRenderEffect->SetRasterizerState(CommonStates::CullNone());
	particleRenderEffect->SetDepthStencilState(CommonStates::DepthRead());
	particleRenderEffect->SetBlendState(CommonStates::AlphaBlend());
	particleRenderEffect->SetRenderTargetFormat(ColorFormat::R11G11B10_Float, DepthFormat::D32);
	particleRenderEffect->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	particleRenderEffect->SetVertexShaderPath("Engine\\ParticleVS");
	particleRenderEffect->SetPixelShaderPath("Engine\\ParticlePS");
	particleRenderEffect->Finalize();
	waitTask = waitTask && particleRenderEffect->loadTask;

	m_particleRenderPass = make_shared<RenderPass>("Particles");

	m_particleRenderMaterial = make_shared<Material>();
	m_particleRenderMaterial->SetName("Particle material");
	m_particleRenderMaterial->SetRenderPass(m_particleRenderPass);
	m_particleRenderMaterial->SetEffect(particleRenderEffect);

	for (uint32_t i = 0; i < 7; ++i)
	{
		m_particleInnerSortCs[i].SetComputeShaderPath("Engine\\ParticleInnerSortCS", waitTask);
	}

	for (uint32_t i = 0; i < 28; ++i)
	{
		m_particleOuterSortCs[i].SetComputeShaderPath("Engine\\ParticleOuterSortCS", waitTask);
	}

	__declspec(align(16)) UINT initialDrawIndirectArgs[4] = { 4, 0, 0, 0 };
	m_drawIndirectArgs.Create("ParticleEffects::DrawIndirectArgs", 1, 4 * sizeof(UINT), initialDrawIndirectArgs);

	__declspec(align(16)) UINT initialDispatchIndirectArgs[6] = { 0, 1, 1, 0, 1, 1 };
	m_finalDispatchIndirectArgs.Create("ParticleEffects::FinalDispatchIndirectArgs", 1, 3 * sizeof(UINT), initialDispatchIndirectArgs);

	m_spriteVertexBuffer.Create("ParticleEffects::SpriteVertexBuffer", MAX_TOTAL_PARTICLES, sizeof(ParticleVertex));
	m_visibleParticleBuffer.Create("ParticleEffects::VisibleParticleBuffer", MAX_TOTAL_PARTICLES, sizeof(ParticleScreenData));
	m_spriteIndexBuffer.Create("ParticleEffects::SpriteIndexBuffer", MAX_TOTAL_PARTICLES, sizeof(UINT));
	m_sortIndirectArgs.Create("ParticleEffects::SortIndirectArgs", 8, 3 * sizeof(UINT));
	m_tileDrawDispatchIndirectArgs.Create("ParticleEffects::DrawPackets_IArgs", 2, 3 * sizeof(UINT), initialDispatchIndirectArgs);

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

	m_binParticles[0].Create("ParticleEffects::BinParticles[0]", particleBinCapacity, sizeof(UINT));
	m_binParticles[1].Create("ParticleEffects::BinParticles[1]", particleBinCapacity, sizeof(UINT));

	m_binCounters[0].Create("ParticleEffects::LargeBinCounters", largeBinsPerRow * largeBinsPerCol, sizeof(UINT));
	m_binCounters[1].Create("ParticleEffects::BinCounters", binsPerRow * binsPerCol, sizeof(UINT));

	m_tileCounters.Create("ParticleEffects::TileCounters", paddedTilesPerRow * paddedTilesPerCol, sizeof(UINT));
	m_tileHitMasks.Create("ParticleEffects::TileHitMasks", paddedTilesPerRow * paddedTilesPerCol, MAX_PARTICLES_PER_BIN / 8);

	m_tileDrawPackets.Create("ParticleEffects::DrawPackets", tilesPerRow * tilesPerCol, sizeof(UINT));
	m_tileFastDrawPackets.Create("ParticleEffects::FastDrawPackets", tilesPerRow * tilesPerCol, sizeof(UINT));

	m_textureArray.CreateArray(64, 64, 16, 4, ColorFormat::BC3_UNorm_sRGB);

	const uint32_t bufferWidth3 = (width + 7) / 8;
	const uint32_t bufferWidth4 = (width + 15) / 16;
	const uint32_t bufferWidth5 = (width + 31) / 32;
	const uint32_t bufferHeight3 = (height + 7) / 8;
	const uint32_t bufferHeight4 = (height + 15) / 16;
	const uint32_t bufferHeight5 = (height + 31) / 32;

	m_minMaxDepth8.Create("ParticleEffects::MinMaxDepth 8x8", bufferWidth3, bufferHeight3, 1, ColorFormat::R32_UInt);
	m_minMaxDepth16.Create("ParticleEffects::MinMaxDepth 16x16", bufferWidth4, bufferHeight4, 1, ColorFormat::R32_UInt);
	m_minMaxDepth32.Create("ParticleEffects::MinMaxDepth 32x32", bufferWidth5, bufferHeight5, 1, ColorFormat::R32_UInt);

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


void ParticleEffectManager::Update(ComputeCommandList& commandList, float timeDelta)
{
	if (!m_enable || !m_initialized || m_particleEffectsActive.empty())
	{
		return;
	}

	if (++m_totalElapsedFrames == m_reproFrame)
	{
		m_pauseSim = true;
	}

	if (m_pauseSim)
	{
		return;
	}

	commandList.PIXBeginEvent("Update particles");

	commandList.ResetCounter(m_spriteVertexBuffer);
	commandList.TransitionResource(m_spriteVertexBuffer, ResourceState::UnorderedAccess);

	for (size_t i = 0; i < m_particleEffectsActive.size(); ++i)
	{
		m_particleEffectsActive[i]->Update(commandList, m_spriteVertexBuffer, timeDelta);

		if (m_particleEffectsActive[i]->GetLifetime() <= m_particleEffectsActive[i]->GetElapsedTime())
		{
			// Erase from vector
			auto iter = m_particleEffectsActive.begin() + i;

			static mutex s_eraseEffectMutex;
			s_eraseEffectMutex.lock();

			m_particleEffectsActive.erase(iter);

			s_eraseEffectMutex.unlock();
		}
	}

	SetFinalBuffers(commandList);

	commandList.PIXEndEvent();
}


void ParticleEffectManager::Render(CommandList& commandList, std::shared_ptr<Camera> camera, ColorBuffer& colorTarget,
	DepthBuffer& depthBuffer, ColorBuffer& linearDepth)
{
	if (!m_enable || !m_initialized || m_particleEffectsActive.empty())
	{
		return;
	}

	auto width = colorTarget.GetWidth();
	auto height = colorTarget.GetHeight();

	assert_msg(width == depthBuffer.GetWidth() &&
		height == depthBuffer.GetHeight() &&
		width == linearDepth.GetWidth() &&
		height == linearDepth.GetHeight(),
		"There is a mismatch in buffer dimensions for rendering particles");

	uint32_t binsPerRow = 4 * DivideByMultiple(width, 4 * BIN_SIZE_X);

	CBChangesPerView cbChangesPerView;
	cbChangesPerView.ViewProj = camera->GetViewProjMatrix();
	cbChangesPerView.InvView = Invert(camera->GetViewMatrix());
	float hCot = camera->GetProjMatrix().GetX().GetX();
	float vCot = camera->GetProjMatrix().GetY().GetY();
	cbChangesPerView.VertCotangent = vCot;
	cbChangesPerView.AspectRatio = hCot / vCot;
	cbChangesPerView.RcpFarZ = 1.0f / camera->GetFarClip();
	cbChangesPerView.InvertZ = camera->GetNearClip() / (camera->GetFarClip() - camera->GetNearClip());
	cbChangesPerView.BufferWidth = static_cast<float>(width);
	cbChangesPerView.BufferHeight = static_cast<float>(height);
	cbChangesPerView.RcpBufferWidth = 1.0f / cbChangesPerView.BufferWidth;
	cbChangesPerView.RcpBufferHeight = 1.0f / cbChangesPerView.BufferHeight;
	cbChangesPerView.BinsPerRow = binsPerRow;
	cbChangesPerView.TileRowPitch = binsPerRow * TILES_PER_BIN_X;
	cbChangesPerView.TilesPerRow = DivideByMultiple(width, TILE_SIZE);
	cbChangesPerView.TilesPerCol = DivideByMultiple(height, TILE_SIZE);

	commandList.PIXBeginEvent("Particles");

	// For now, UAV load support for R11G11B10 is required to read-modify-write the color buffer, but
	// the compositing could be deferred.
	auto& deviceManager = DeviceManager::GetInstance();
	warn_once_if(m_enableTiledRendering && !deviceManager.SupportsTypedUAVLoad_R11G11B10_FLOAT(),
		"Unable to composite tiled particles without support for R11G11B10F UAV loads");
	m_enableTiledRendering = m_enableTiledRendering && deviceManager.SupportsTypedUAVLoad_R11G11B10_FLOAT();

	if (m_enableTiledRendering)
	{
		auto& compCommandList = commandList.GetComputeCommandList();
		compCommandList.TransitionResource(colorTarget, ResourceState::UnorderedAccess);
		compCommandList.TransitionResource(m_binCounters[0], ResourceState::UnorderedAccess);
		compCommandList.TransitionResource(m_binCounters[1], ResourceState::UnorderedAccess);

		compCommandList.ClearUAV(m_binCounters[0]);
		compCommandList.ClearUAV(m_binCounters[1]);

		RenderTiles(compCommandList, cbChangesPerView, colorTarget, depthBuffer, linearDepth);

		compCommandList.InsertUAVBarrier(colorTarget);
	}
	else
	{
		auto& gfxCommandList = commandList.GetGraphicsCommandList();

		RenderSprites(gfxCommandList, cbChangesPerView, colorTarget, depthBuffer, linearDepth);
	}

	commandList.PIXEndEvent();
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

	string fullPath = Paths::GetInstance().TextureDir() + name;
	auto texture = Texture::Load(fullPath, true);
	texture->loadTask.wait();

	GpuResource& particleTexture = *texture;
	CommandList::InitializeTextureArraySlice(m_textureArray, textureID, particleTexture);
}


void ParticleEffectManager::SetFinalBuffers(ComputeCommandList& commandList)
{
	commandList.PIXBeginEvent("Set final buffers");

	commandList.TransitionResource(m_spriteVertexBuffer, ResourceState::GenericRead);
	commandList.TransitionResource(m_finalDispatchIndirectArgs, ResourceState::UnorderedAccess);
	commandList.TransitionResource(m_drawIndirectArgs, ResourceState::UnorderedAccess);

	m_particleFinalDispatchIndirectArgsCs.GetResource("g_FinalInstanceCounter")->SetSRVImmediate(*m_spriteVertexBuffer.GetCounterSRV(commandList));
	m_particleFinalDispatchIndirectArgsCs.GetResource("g_NumThreadGroups")->SetUAVImmediate(m_finalDispatchIndirectArgs);
	m_particleFinalDispatchIndirectArgsCs.GetResource("g_DrawIndirectArgs")->SetUAVImmediate(m_drawIndirectArgs);

	m_particleFinalDispatchIndirectArgsCs.Dispatch(commandList, 1, 1, 1);
	m_particleFinalDispatchIndirectArgsCs.UnbindUAVs(commandList);

	commandList.PIXEndEvent();
}


void ParticleEffectManager::RenderTiles(ComputeCommandList& commandList, const CBChangesPerView& cbData, ColorBuffer& colorTarget,
	DepthBuffer& depthTarget, ColorBuffer& linearDepth)
{
	auto screenWidth = colorTarget.GetWidth();
	auto screenHeight = colorTarget.GetHeight();

	auto& deviceManager = DeviceManager::GetInstance();
	assert_msg(colorTarget.GetFormat() == DXGI_FORMAT_R32_UINT || deviceManager.SupportsTypedUAVLoad_R11G11B10_FLOAT(),
		"Without typed UAV loads, tiled particles must render to a R32_UINT buffer");

	// Compute depth bounds
	{
		commandList.PIXBeginEvent("Compute depth bounds");

		commandList.TransitionResource(linearDepth, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_minMaxDepth8, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_minMaxDepth16, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_minMaxDepth32, ResourceState::UnorderedAccess);

		m_particleDepthBoundsCs.GetConstantBuffer("CBChangesPerView")->SetDataImmediate(
			sizeof(CBChangesPerView),
			reinterpret_cast<const byte*>(&cbData));
		m_particleDepthBoundsCs.GetParameter("LogTilesPerLargeBin")->SetValueImmediate(DirectX::XMUINT2(5, 4));
		m_particleDepthBoundsCs.GetResource("g_Input")->SetSRVImmediate(linearDepth);
		m_particleDepthBoundsCs.GetResource("g_Output8")->SetUAVImmediate(m_minMaxDepth8);
		m_particleDepthBoundsCs.GetResource("g_Output16")->SetUAVImmediate(m_minMaxDepth16);
		m_particleDepthBoundsCs.GetResource("g_Output32")->SetUAVImmediate(m_minMaxDepth32);

		m_particleDepthBoundsCs.Dispatch2D(commandList, screenWidth, screenHeight, 32, 32);

		commandList.PIXEndEvent();
	}

	// Culling and sorting
	{
		commandList.PIXBeginEvent("Culling and sorting");

		// The first step inserts each particle into all of the large bins it intersects.  Large bins
		// are 512x256.
		commandList.TransitionResource(m_spriteVertexBuffer, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_drawIndirectArgs, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_finalDispatchIndirectArgs, ResourceState::IndirectArgument);
		commandList.TransitionResource(m_binParticles[0], ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_binCounters[0], ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_visibleParticleBuffer, ResourceState::UnorderedAccess);

		m_particleLargeBinCullingCs.GetConstantBuffer("CBChangesPerView")->SetDataImmediate(
			sizeof(CBChangesPerView),
			reinterpret_cast<const byte*>(&cbData));
		m_particleLargeBinCullingCs.GetResource("g_LargeBinParticles")->SetUAVImmediate(m_binParticles[0]);
		m_particleLargeBinCullingCs.GetResource("g_LargeBinCounters")->SetUAVImmediate(m_binCounters[0]);
		m_particleLargeBinCullingCs.GetResource("g_VisibleParticles")->SetUAVImmediate(m_visibleParticleBuffer);
		m_particleLargeBinCullingCs.GetResource("g_VertexBuffer")->SetSRVImmediate(m_spriteVertexBuffer);
		m_particleLargeBinCullingCs.GetResource("g_VertexCount")->SetSRVImmediate(m_drawIndirectArgs);

		m_particleLargeBinCullingCs.DispatchIndirect(commandList, m_finalDispatchIndirectArgs);

		// The second step refines the binning by inserting particles into the appropriate small bins.
		// Small bins are 128x64.
		commandList.TransitionResource(m_visibleParticleBuffer, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_binParticles[0], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_binCounters[0], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_binParticles[1], ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_binCounters[1], ResourceState::UnorderedAccess);

		m_particleBinCullingCs.GetParameter("LogTilesPerBin")->SetValueImmediate(DirectX::XMUINT2(3, 2));
		m_particleBinCullingCs.GetConstantBuffer("CBChangesPerView")->SetDataImmediate(
			sizeof(CBChangesPerView),
			reinterpret_cast<const byte*>(&cbData));
		m_particleBinCullingCs.GetResource("g_VisibleParticles")->SetSRVImmediate(m_visibleParticleBuffer);
		m_particleBinCullingCs.GetResource("g_LargeBinParticles")->SetSRVImmediate(m_binParticles[0]);
		m_particleBinCullingCs.GetResource("g_LargeBinCounters")->SetSRVImmediate(m_binCounters[0]);
		m_particleBinCullingCs.GetResource("g_BinParticles")->SetUAVImmediate(m_binParticles[1]);
		m_particleBinCullingCs.GetResource("g_BinCounters")->SetUAVImmediate(m_binCounters[1]);

		m_particleBinCullingCs.Dispatch2D(commandList, screenWidth, screenHeight, 4 * BIN_SIZE_X, 4 * BIN_SIZE_Y);

		// The final sorting step will perform a bitonic sort on each bin's particles (front to
		// back).  Afterward, it will generate a bitmap for each tile indicating which of the bin's
		// particles occupy the tile.  This allows each tile to iterate over a sorted list of particles
		// ignoring the ones that do not intersect.
		commandList.FillBuffer(m_tileDrawDispatchIndirectArgs, 0, 0, sizeof(uint32_t));
		commandList.FillBuffer(m_tileDrawDispatchIndirectArgs, 12, 0, sizeof(uint32_t));

		commandList.TransitionResource(m_binParticles[0], ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_tileHitMasks, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_tileDrawPackets, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_tileFastDrawPackets, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_tileDrawDispatchIndirectArgs, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_binParticles[1], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_binCounters[1], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_minMaxDepth8, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_minMaxDepth16, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_minMaxDepth32, ResourceState::NonPixelShaderResource);

		m_particleTileCullingCs.GetConstantBuffer("CBChangesPerView")->SetDataImmediate(
			sizeof(CBChangesPerView),
			reinterpret_cast<const byte*>(&cbData));

		m_particleTileCullingCs.GetResource("g_BinParticles")->SetSRVImmediate(m_binParticles[1]);
		m_particleTileCullingCs.GetResource("g_BinCounters")->SetSRVImmediate(m_binCounters[1]);
		m_particleTileCullingCs.GetResource("g_DepthBounds")->SetSRVImmediate(TILE_SIZE == 16 ? m_minMaxDepth16 : m_minMaxDepth32);
		m_particleTileCullingCs.GetResource("g_VisibleParticles")->SetSRVImmediate(m_visibleParticleBuffer);

		m_particleTileCullingCs.GetResource("g_SortedParticles")->SetUAVImmediate(m_binParticles[0]);
		m_particleTileCullingCs.GetResource("g_TileHitMasks")->SetUAVImmediate(m_tileHitMasks);
		m_particleTileCullingCs.GetResource("g_DrawPackets")->SetUAVImmediate(m_tileDrawPackets);
		m_particleTileCullingCs.GetResource("g_FastDrawPackets")->SetUAVImmediate(m_tileFastDrawPackets);
		m_particleTileCullingCs.GetResource("g_DrawPacketCount")->SetUAVImmediate(m_tileDrawDispatchIndirectArgs);

		m_particleTileCullingCs.Dispatch2D(commandList, screenWidth, screenHeight, BIN_SIZE_X, BIN_SIZE_Y);

		commandList.PIXEndEvent();
	}

	// Tiled rendering
	{
		commandList.PIXBeginEvent("Tiled rendering");

		commandList.TransitionResource(m_tileDrawDispatchIndirectArgs, ResourceState::IndirectArgument);
		commandList.TransitionResource(colorTarget, ResourceState::UnorderedAccess);
		commandList.TransitionResource(linearDepth, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_binParticles[0], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_tileHitMasks, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_tileDrawPackets, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_tileFastDrawPackets, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_textureArray, ResourceState::NonPixelShaderResource);

		m_particleTileRenderSlowCs[m_tiledRes].GetConstantBuffer("CBChangesPerView")->SetDataImmediate(
			sizeof(CBChangesPerView),
			reinterpret_cast<const byte*>(&cbData));

		// Slow pass
		m_particleTileRenderSlowCs[m_tiledRes].GetResource("g_OutputColorBuffer")->SetUAVImmediate(colorTarget);
		m_particleTileRenderSlowCs[m_tiledRes].GetResource("g_VisibleParticles")->SetSRVImmediate(m_visibleParticleBuffer);
		m_particleTileRenderSlowCs[m_tiledRes].GetResource("g_HitMask")->SetSRVImmediate(m_tileHitMasks);
		m_particleTileRenderSlowCs[m_tiledRes].GetResource("g_TexArray")->SetSRVImmediate(m_textureArray);
		m_particleTileRenderSlowCs[m_tiledRes].GetResource("g_InputDepthBuffer")->SetSRVImmediate(linearDepth);
		m_particleTileRenderSlowCs[m_tiledRes].GetResource("g_SortedParticles")->SetSRVImmediate(m_binParticles[0]);
		m_particleTileRenderSlowCs[m_tiledRes].GetResource("g_DrawPackets")->SetSRVImmediate(m_tileDrawPackets);
		m_particleTileRenderSlowCs[m_tiledRes].GetResource("g_TileDepthBounds")->SetSRVImmediate(TILE_SIZE == 16 ? m_minMaxDepth16 : m_minMaxDepth32);

		m_particleTileRenderSlowCs[m_tiledRes].GetParameter("gDynamicResLevel")->SetValueImmediate(m_dynamicResLevel);
		m_particleTileRenderSlowCs[m_tiledRes].GetParameter("gMipBias")->SetValueImmediate(m_mipBias);

		m_particleTileRenderSlowCs[m_tiledRes].DispatchIndirect(commandList, m_tileDrawDispatchIndirectArgs);

		// Fast pass
		m_particleTileRenderFastCs[m_tiledRes].GetConstantBuffer("CBChangesPerView")->SetDataImmediate(
			sizeof(CBChangesPerView),
			reinterpret_cast<const byte*>(&cbData));

		m_particleTileRenderFastCs[m_tiledRes].GetResource("g_OutputColorBuffer")->SetUAVImmediate(colorTarget);
		m_particleTileRenderFastCs[m_tiledRes].GetResource("g_VisibleParticles")->SetSRVImmediate(m_visibleParticleBuffer);
		m_particleTileRenderFastCs[m_tiledRes].GetResource("g_HitMask")->SetSRVImmediate(m_tileHitMasks);
		m_particleTileRenderFastCs[m_tiledRes].GetResource("g_TexArray")->SetSRVImmediate(m_textureArray);
		m_particleTileRenderFastCs[m_tiledRes].GetResource("g_InputDepthBuffer")->SetSRVImmediate(linearDepth);
		m_particleTileRenderFastCs[m_tiledRes].GetResource("g_SortedParticles")->SetSRVImmediate(m_binParticles[0]);
		m_particleTileRenderFastCs[m_tiledRes].GetResource("g_DrawPackets")->SetSRVImmediate(m_tileFastDrawPackets);
		m_particleTileRenderFastCs[m_tiledRes].GetResource("g_TileDepthBounds")->SetSRVImmediate(TILE_SIZE == 16 ? m_minMaxDepth16 : m_minMaxDepth32);

		m_particleTileRenderFastCs[m_tiledRes].GetParameter("gDynamicResLevel")->SetValueImmediate(m_dynamicResLevel);
		m_particleTileRenderFastCs[m_tiledRes].GetParameter("gMipBias")->SetValueImmediate(m_mipBias);

		m_particleTileRenderFastCs[m_tiledRes].DispatchIndirect(commandList, m_tileDrawDispatchIndirectArgs, 12);

		commandList.PIXEndEvent();
	}
}


void ParticleEffectManager::RenderSprites(GraphicsCommandList& commandList, const CBChangesPerView& cbData, ColorBuffer& colorTarget,
	DepthBuffer& depthTarget, ColorBuffer& linearDepth)
{
	commandList.PIXBeginEvent("Render sprites");

	if (m_enableSpriteSort)
	{
		commandList.PIXBeginEvent("Sort particles");

		auto& compCommandList = commandList.GetComputeCommandList();

		compCommandList.TransitionResource(m_spriteVertexBuffer, ResourceState::NonPixelShaderResource);
		compCommandList.TransitionResource(m_sortIndirectArgs, ResourceState::UnorderedAccess);
		compCommandList.TransitionResource(m_drawIndirectArgs, ResourceState::NonPixelShaderResource);

		m_particleSortIndirectArgsCs.GetResource("g_ActiveParticlesCount")->SetSRVImmediate(m_drawIndirectArgs);
		m_particleSortIndirectArgsCs.GetResource("g_IndirectArgsBuffer")->SetUAVImmediate(m_sortIndirectArgs);
		
		m_particleSortIndirectArgsCs.Dispatch(compCommandList, 1, 1, 1);
		m_particleSortIndirectArgsCs.UnbindUAVs(compCommandList);

		compCommandList.TransitionResource(m_sortIndirectArgs, ResourceState::IndirectArgument);
		compCommandList.TransitionResource(m_spriteIndexBuffer, ResourceState::UnorderedAccess);
		m_particlePreSortCs.GetResource("g_VertexBuffer")->SetSRVImmediate(m_spriteVertexBuffer);
		m_particlePreSortCs.GetResource("g_VertexCount")->SetSRVImmediate(m_drawIndirectArgs);
		m_particlePreSortCs.GetResource("g_SortBuffer")->SetUAVImmediate(m_spriteIndexBuffer);

		m_particlePreSortCs.DispatchIndirect(compCommandList, m_sortIndirectArgs);
		m_particlePreSortCs.UnbindUAVs(compCommandList);

		compCommandList.InsertUAVBarrier(m_spriteIndexBuffer);

		commandList.PIXEndEvent();
	}

	Rectangle scissor;
	scissor.left = 0;
	scissor.top = 0;
	scissor.right = colorTarget.GetWidth();
	scissor.bottom = colorTarget.GetHeight();

	Viewport viewport;
	viewport.topLeftX = 0.0f;
	viewport.topLeftY = 0.0f;
	viewport.width = static_cast<float>(colorTarget.GetWidth());
	viewport.height = static_cast<float>(colorTarget.GetHeight());
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	commandList.TransitionResource(m_spriteVertexBuffer, ResourceState::NonPixelShaderResource);
	commandList.TransitionResource(m_drawIndirectArgs, ResourceState::IndirectArgument);
	commandList.TransitionResource(m_textureArray, ResourceState::PixelShaderResource);
	commandList.TransitionResource(linearDepth, ResourceState::PixelShaderResource);
	commandList.TransitionResource(m_spriteIndexBuffer, ResourceState::PixelShaderResource);
	commandList.TransitionResource(colorTarget, ResourceState::RenderTarget);
	commandList.TransitionResource(depthTarget, ResourceState::DepthRead);

	commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList.SetRenderTarget(colorTarget, depthTarget, true);
	commandList.SetViewportAndScissor(viewport, scissor);

	m_particleRenderMaterial->GetConstantBuffer("CBChangesPerView")->SetDataImmediate(
		sizeof(CBChangesPerView),
		reinterpret_cast<const byte*>(&cbData));
	m_particleRenderMaterial->GetResource("g_VertexBuffer")->SetSRVImmediate(m_spriteVertexBuffer);
	m_particleRenderMaterial->GetResource("ColorTex")->SetSRVImmediate(m_textureArray);
	m_particleRenderMaterial->GetResource("g_IndexBuffer")->SetSRVImmediate(m_spriteIndexBuffer);
	m_particleRenderMaterial->GetResource("LinearDepthTex")->SetSRVImmediate(linearDepth);

	commandList.DrawIndirect(m_drawIndirectArgs);

	commandList.PIXEndEvent();
}


void ParticleEffectManager::BitonicSort(ComputeCommandList& commandList)
{
	uint32_t indirectArgsOffset = 12;
	uint32_t innerIndex = 0;
	uint32_t outerIndex = 0;

	for (uint32_t k = 4096; k <= 256 * 1024; k *= 2)
	{
		for (uint32_t j = k / 2; j >= 2048; j /= 2)
		{
			m_particleOuterSortCs[outerIndex].GetParameter("k")->SetValueImmediate(k);
			m_particleOuterSortCs[outerIndex].GetParameter("j")->SetValueImmediate(j);
			m_particleOuterSortCs[outerIndex].GetResource("g_SortBuffer")->SetUAVImmediate(m_spriteIndexBuffer);

			m_particleOuterSortCs[outerIndex].DispatchIndirect(commandList, m_sortIndirectArgs, indirectArgsOffset);
			commandList.InsertUAVBarrier(m_spriteIndexBuffer);

			++outerIndex;
		}

		m_particleInnerSortCs[innerIndex].GetParameter("k")->SetValueImmediate(k);
		m_particleInnerSortCs[innerIndex].GetResource("g_SortBuffer")->SetUAVImmediate(m_spriteIndexBuffer);

		m_particleInnerSortCs[innerIndex].DispatchIndirect(commandList, m_sortIndirectArgs, indirectArgsOffset);
		commandList.InsertUAVBarrier(m_spriteIndexBuffer);

		++innerIndex;
		indirectArgsOffset += 12;
	}
}
#endif