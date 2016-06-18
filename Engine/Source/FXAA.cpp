// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from FXAA.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "FXAA.h"

#include "ColorBuffer.h"
#include "CommandList.h"
#include "ComputeKernel.h"
#include "ComputeParameter.h"
#include "ComputeResource.h"
#include "GpuBuffer.h"


using namespace Kodiak;
using namespace std;
using namespace DirectX;


FXAA::FXAA()
	: SceneColorBuffer(m_sceneColorBuffer)
	, LumaBuffer(m_lumaBuffer)
	, UsePrecomputedLuma(m_usePreComputedLuma)
	, DebugDraw(m_debugDraw)
	, ContrastThreshold(m_contrastThreshold)
	, SubpixelRemoval(m_subpixelRemoval)
{}


void FXAA::Initialize(uint32_t width, uint32_t height)
{
	m_pass1HdrCs = make_shared<ComputeKernel>();
	m_pass1HdrCs->SetComputeShaderPath("Engine\\FXAAPass1_Luma_CS");
	auto waitTask = m_pass1HdrCs->loadTask;

	m_pass1LdrCs = make_shared<ComputeKernel>();
	m_pass1LdrCs->SetComputeShaderPath("Engine\\FXAAPass1_RGB_CS");
	waitTask = waitTask && m_pass1LdrCs->loadTask;

	m_resolveWorkCs = make_shared<ComputeKernel>();
	m_resolveWorkCs->SetComputeShaderPath("Engine\\FXAAResolveWorkQueueCS");
	waitTask = waitTask && m_resolveWorkCs->loadTask;

	m_pass2HDebugCs = make_shared<ComputeKernel>();
	m_pass2HDebugCs->SetComputeShaderPath("Engine\\FXAAPass2HDebugCS");
	waitTask = waitTask && m_pass2HDebugCs->loadTask;

	m_pass2HCs = make_shared<ComputeKernel>();
	m_pass2HCs->SetComputeShaderPath("Engine\\FXAAPass2HCS");
	waitTask = waitTask && m_pass2HCs->loadTask;

	m_fxaaWorkQueueH = make_shared<StructuredBuffer>();
	m_fxaaWorkQueueH->Create("FXAA Horizontal Work Queue", 512 * 1024, 4);

	m_fxaaWorkQueueV = make_shared<StructuredBuffer>();
	m_fxaaWorkQueueV->Create("FXAA Vertical Work Queue", 512 * 1024, 4);

	m_fxaaColorQueueH = make_shared<TypedBuffer>(DXGI_FORMAT_R11G11B10_FLOAT);
	m_fxaaColorQueueH->Create("FXAA Horizontal Color Queue", 512 * 1024, 4);

	m_fxaaColorQueueV = make_shared<TypedBuffer>(DXGI_FORMAT_R11G11B10_FLOAT);
	m_fxaaColorQueueV->Create("FXAA Vertical Color Queue", 512 * 1024, 4);

	m_indirectParameters = make_shared<IndirectArgsBuffer>();
	__declspec(align(16)) const uint32_t initArgs[6] = { 0, 1, 1, 0, 1, 1 };
	m_indirectParameters->Create("FXAA Indirect Parameters", 2, 3 * sizeof(uint32_t), initArgs);

	waitTask.wait();
}


void FXAA::Render(GraphicsCommandList* commandList)
{
	commandList->PIXBeginEvent("FXAA");

	{
		commandList->PIXBeginEvent("Pass 1");

		commandList->TransitionResource(*m_sceneColorBuffer, ResourceState::NonPixelShaderResource);
		commandList->TransitionResource(*m_fxaaWorkQueueH, ResourceState::UnorderedAccess);
		commandList->TransitionResource(*m_fxaaWorkQueueV, ResourceState::UnorderedAccess);
		commandList->TransitionResource(*m_fxaaColorQueueH, ResourceState::UnorderedAccess);
		commandList->TransitionResource(*m_fxaaColorQueueV, ResourceState::UnorderedAccess);

		auto computeKernel = m_usePreComputedLuma ? m_pass1HdrCs : m_pass1LdrCs;

		computeKernel->GetResource("HWork")->SetUAVImmediate(m_fxaaWorkQueueH);
		computeKernel->GetResource("VWork")->SetUAVImmediate(m_fxaaWorkQueueV);
		computeKernel->GetResource("HColor")->SetUAVImmediate(m_fxaaColorQueueH);
		computeKernel->GetResource("HColor")->SetUAVImmediate(m_fxaaColorQueueV);
		computeKernel->GetResource("Color")->SetSRVImmediate(m_sceneColorBuffer);

		computeKernel->GetParameter("RcpTextureSize")->SetValueImmediate(XMFLOAT2(1.0f / m_sceneColorBuffer->GetWidth(), 1.0f / m_sceneColorBuffer->GetHeight()));
		computeKernel->GetParameter("ContrastThreshold")->SetValueImmediate(m_contrastThreshold);
		computeKernel->GetParameter("SubpixelRemoval")->SetValueImmediate(m_subpixelRemoval);

		if (m_usePreComputedLuma)
		{
			commandList->TransitionResource(*m_lumaBuffer, ResourceState::NonPixelShaderResource);
			computeKernel->GetResource("Luma")->SetSRVImmediate(m_lumaBuffer);
		}
		else
		{
			commandList->TransitionResource(*m_lumaBuffer, ResourceState::UnorderedAccess);
			computeKernel->GetResource("Luma")->SetUAVImmediate(m_lumaBuffer);
		}

		auto computeCommandList = commandList->GetComputeCommandList();
		computeKernel->Dispatch2D(computeCommandList, m_sceneColorBuffer->GetWidth(), m_sceneColorBuffer->GetHeight());

		commandList->PIXEndEvent();
	}

	{
		commandList->PIXBeginEvent("Pass 2");

		// The next phase involves converting the work queues to DispatchIndirect parameters.
		// The queues are also padded out to 64 elements to simplify the final consume logic.

		commandList->TransitionResource(*m_indirectParameters, ResourceState::UnorderedAccess);

#if DX11
		// This preserves the structured buffers' hidden counter values from the previous phase
		m_fxaaWorkQueueH->SetCounterInitialValue((uint32_t)-1);
		m_fxaaWorkQueueV->SetCounterInitialValue((uint32_t)-1);
#endif

		m_resolveWorkCs->GetResource("IndirectParams")->SetUAVImmediate(m_indirectParameters);
		m_resolveWorkCs->GetResource("WorkQueueH")->SetUAVImmediate(m_fxaaWorkQueueH);
		m_resolveWorkCs->GetResource("WorkQueueV")->SetUAVImmediate(m_fxaaWorkQueueV);

#if DX12
		// Read the structured buffers' counter buffers directly
		m_resolveWorkCs->GetResource("WorkCounterH")->SetUAVImmediate(m_fxaaWorkQueueH->GetCounterBuffer());
		m_resolveWorkCs->GetResource("WorkCounterV")->SetUAVImmediate(m_fxaaWorkQueueV->GetCounterBuffer());
#endif

		auto computeCommandList = commandList->GetComputeCommandList();
		m_resolveWorkCs->Dispatch(computeCommandList, 1, 1, 1);

		commandList->TransitionResource(*m_sceneColorBuffer, ResourceState::UnorderedAccess);
		commandList->TransitionResource(*m_indirectParameters, ResourceState::IndirectArgument);
		commandList->TransitionResource(*m_fxaaWorkQueueH, ResourceState::NonPixelShaderResource);
		commandList->TransitionResource(*m_fxaaWorkQueueV, ResourceState::NonPixelShaderResource);
		commandList->TransitionResource(*m_fxaaColorQueueH, ResourceState::NonPixelShaderResource);
		commandList->TransitionResource(*m_fxaaColorQueueV, ResourceState::NonPixelShaderResource);

		auto computeKernel = m_debugDraw ? m_pass2HDebugCs : m_pass2HCs;

		commandList->PIXEndEvent();
	}

	commandList->PIXEndEvent();
}