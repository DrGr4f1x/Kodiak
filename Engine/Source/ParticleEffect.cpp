// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleEffect.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//                      James Stanard
//

#include "Stdafx.h"

#include "ParticleEffect.h"

#include "CommandList.h"
#include "ComputeConstantBuffer.h"
#include "ComputeKernel.h"
#include "ComputeResource.h"
#include "ComputeParameter.h"
#include "Random.h"


using namespace Kodiak;
using namespace std;
using namespace Math;
using namespace DirectX;


ParticleEffect::ParticleEffect(ParticleEffectProperties* effectProperties)
{
	m_elapsedTime = 0.0;
	m_effectProperties = *effectProperties;
}


inline static Color RandColor(Color c0, Color c1)
{
	// We might want to find min and max of each channel rather than assuming c0 <= c1
	return Color(
		g_rng.NextFloat(c0.R(), c1.R()),
		g_rng.NextFloat(c0.G(), c1.G()),
		g_rng.NextFloat(c0.B(), c1.B()),
		g_rng.NextFloat(c0.A(), c1.A())
	);
}


inline static XMFLOAT3 RandSpread(const XMFLOAT3& s)
{
	// We might want to find min and max of each channel rather than assuming c0 <= c1
	return XMFLOAT3(
		g_rng.NextFloat(-s.x, s.x),
		g_rng.NextFloat(-s.y, s.y),
		g_rng.NextFloat(-s.z, s.z)
	);
}


void ParticleEffect::LoadDeviceResources()
{
	m_updateCs = make_shared<ComputeKernel>();
	m_updateCs->SetComputeShaderPath("Engine\\ParticleUpdateCS");
	auto waitTask = m_updateCs->loadTask;

	m_particleSpawnCs = make_shared<ComputeKernel>();
	m_particleSpawnCs->SetComputeShaderPath("Engine\\ParticleSpawnCS");
	waitTask = waitTask && m_particleSpawnCs->loadTask;

	m_dispatchIndirectArgsCs = make_shared<ComputeKernel>();
	m_dispatchIndirectArgsCs->SetComputeShaderPath("Engine\\ParticleDispatchIndirectArgsCS");
	waitTask = waitTask && m_dispatchIndirectArgsCs->loadTask;

	m_originalEffectProperties = m_effectProperties; // In case we want to reset
	
	// Fill particle spawn data buffer
	ParticleSpawnData* pSpawnData = (ParticleSpawnData*)_malloca(m_effectProperties.EmitProperties.MaxParticles * sizeof(ParticleSpawnData));

	for (UINT i = 0; i < m_effectProperties.EmitProperties.MaxParticles; ++i)
	{
		ParticleSpawnData& SpawnData = pSpawnData[i];
		SpawnData.AgeRate = 1.0f / g_rng.NextFloat(m_effectProperties.LifeMinMax.x, m_effectProperties.LifeMinMax.y);
		float horizontalAngle = g_rng.NextFloat(XM_2PI);
		float horizontalVelocity = g_rng.NextFloat(m_effectProperties.Velocity.GetX(), m_effectProperties.Velocity.GetY());
		SpawnData.Velocity.x = horizontalVelocity * cos(horizontalAngle);
		SpawnData.Velocity.y = g_rng.NextFloat(m_effectProperties.Velocity.GetZ(), m_effectProperties.Velocity.GetW());
		SpawnData.Velocity.z = horizontalVelocity * sin(horizontalAngle);

		SpawnData.SpreadOffset = RandSpread(m_effectProperties.Spread);

		SpawnData.StartSize = g_rng.NextFloat(m_effectProperties.Size.GetX(), m_effectProperties.Size.GetY());
		SpawnData.EndSize = g_rng.NextFloat(m_effectProperties.Size.GetZ(), m_effectProperties.Size.GetW());
		SpawnData.StartColor = RandColor(m_effectProperties.MinStartColor, m_effectProperties.MaxStartColor);
		SpawnData.EndColor = RandColor(m_effectProperties.MinEndColor, m_effectProperties.MaxEndColor);
		SpawnData.Mass = g_rng.NextFloat(m_effectProperties.MassMinMax.x, m_effectProperties.MassMinMax.y);
		SpawnData.RotationSpeed = g_rng.NextFloat(); //todo
		SpawnData.Random = g_rng.NextFloat();
	}

	m_randomStateBuffer = make_shared<StructuredBuffer>();
	m_randomStateBuffer->Create("ParticleSystem::SpawnDataBuffer", m_effectProperties.EmitProperties.MaxParticles, sizeof(ParticleSpawnData), pSpawnData);
	_freea(pSpawnData);

	m_stateBuffers[0] = make_shared<StructuredBuffer>();
	m_stateBuffers[0]->Create("ParticleSystem::Buffer0", m_effectProperties.EmitProperties.MaxParticles, sizeof(ParticleMotion));

	m_stateBuffers[1] = make_shared<StructuredBuffer>();
	m_stateBuffers[1]->Create("ParticleSystem::Buffer1", m_effectProperties.EmitProperties.MaxParticles, sizeof(ParticleMotion));
	m_currentStateBuffer = 0;

	// DispatchIndirect args buffer / number of thread groups
	__declspec(align(16)) UINT DispatchIndirectData[3] = { 0, 1, 1 };
	m_dispatchIndirectArgs = make_shared<IndirectArgsBuffer>();
	m_dispatchIndirectArgs->Create("ParticleSystem::DispatchIndirectArgs", 1, 3 * sizeof(uint32_t), DispatchIndirectData);

	waitTask.wait();
}


void ParticleEffect::Update(ComputeCommandList& commandList, shared_ptr<StructuredBuffer> vertexBuffer, float timeDelta)
{
	m_elapsedTime += timeDelta;
	m_effectProperties.EmitProperties.LastEmitPosW = m_effectProperties.EmitProperties.EmitPosW;

	//m_effectProperties.EmitProperties.EmitPosW = XMFLOAT3(ComputeConstants.EmitPosW.x + 1.0f * float(GameInput::IsPressed(GameInput::kBButton)), ComputeConstants.EmitPosW.y + 1.0f * float(GameInput::IsPressed(GameInput::kYButton)), ComputeConstants.EmitPosW.z - 1.0f * float(GameInput::IsPressed(GameInput::kAButton)));//
	//m_effectProperties.EmitProperties.EmitPosW.x += m_effectProperties.DirectionIncrement.x;
	//m_effectProperties.EmitProperties.EmitPosW.y += m_effectProperties.DirectionIncrement.y;
	//m_effectProperties.EmitProperties.EmitPosW.z += m_effectProperties.DirectionIncrement.z;


	// CPU side random num gen
	for (uint32_t i = 0; i < 64; ++i)
	{
		uint32_t random = (UINT)g_rng.NextInt(m_effectProperties.EmitProperties.MaxParticles - 1);
		m_effectProperties.EmitProperties.RandIndex[i].x = random;
	}

	m_updateCs->GetConstantBuffer("EmissionProperties")->SetDataImmediate(
		sizeof(EmissionProperties),
		reinterpret_cast<const byte*>(&m_effectProperties.EmitProperties));

	commandList.TransitionResource(*m_stateBuffers[m_currentStateBuffer], ResourceState::NonPixelShaderResource);
	m_updateCs->GetResource("g_ResetData")->SetSRVImmediate(m_randomStateBuffer);
	m_updateCs->GetResource("g_InputBuffer")->SetSRVImmediate(m_stateBuffers[m_currentStateBuffer]);
	m_updateCs->GetResource("g_VertexBuffer")->SetUAVImmediate(vertexBuffer);
	m_updateCs->GetParameter("gElapsedTime")->SetValueImmediate(timeDelta);
	m_currentStateBuffer ^= 1;

	commandList.ResetCounter(*m_stateBuffers[m_currentStateBuffer]);

	commandList.TransitionResource(*m_stateBuffers[m_currentStateBuffer], ResourceState::UnorderedAccess);
	commandList.TransitionResource(*m_dispatchIndirectArgs, ResourceState::IndirectArgument);
	
	m_updateCs->GetResource("g_OutputBuffer")->SetUAVImmediate(m_stateBuffers[m_currentStateBuffer]);
	m_updateCs->DispatchIndirect(&commandList, *m_dispatchIndirectArgs, 0);

	// Why need a barrier here so long as we are artificially clamping particle count.  This allows living
	// particles to take precedence over new particles.  The current system always spawns a multiple of 64
	// particles (To Be Fixed) until the total particle count reaches maximum.
	commandList.InsertUAVBarrier(*m_stateBuffers[m_currentStateBuffer]);

	// Spawn to replace dead ones 
	m_particleSpawnCs->GetResource("g_ResetData")->SetSRVImmediate(m_randomStateBuffer);
	UINT NumSpawnThreads = (UINT)(m_effectProperties.EmitRate * timeDelta);
	m_particleSpawnCs->Dispatch(&commandList, (NumSpawnThreads + 63) / 64, 1, 1);

	// Output number of thread groups into m_DispatchIndirectArgs	
	commandList.TransitionResource(*m_dispatchIndirectArgs, ResourceState::UnorderedAccess);
	commandList.TransitionResource(*m_stateBuffers[m_currentStateBuffer], ResourceState::NonPixelShaderResource);

#if DX12
	commandList.TransitionResource(*m_stateBuffers[m_currentStateBuffer]->GetCounterBuffer(), ResourceState::GenericRead);
#elif DX11
	// This preserves the structured buffers' hidden counter values from the previous phase
	m_stateBuffers[m_currentStateBuffer]->SetCounterInitialValue((uint32_t)-1);
	commandList.CopyCounter(*m_stateBuffers[m_currentStateBuffer]->GetCounterBuffer(), 8, *m_stateBuffers[m_currentStateBuffer]);
#endif
	m_dispatchIndirectArgsCs->GetResource("g_ParticleInstance")->SetSRVImmediate(m_stateBuffers[m_currentStateBuffer]->GetCounterBuffer());
	m_dispatchIndirectArgsCs->GetResource("g_NumThreadGroups")->SetUAVImmediate(m_dispatchIndirectArgs);
	m_dispatchIndirectArgsCs->Dispatch(&commandList, 1, 1, 1);
}


void ParticleEffect::Reset()
{
	m_effectProperties = m_originalEffectProperties;
}