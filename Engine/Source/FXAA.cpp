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
#include "ComputeParameter.h"
#include "ComputeResource.h"
#include "DeviceManager.h"


using namespace Kodiak;
using namespace std;
using namespace DirectX;


FXAA::FXAA()
	: SceneColorBuffer(m_sceneColorBuffer)
	, PostEffectsBuffer(m_postEffectsBuffer)
	, LumaBuffer(m_lumaBuffer)
	, UsePrecomputedLuma(m_usePreComputedLuma)
	, DebugDraw(m_debugDraw)
	, ContrastThreshold(m_contrastThreshold)
	, SubpixelRemoval(m_subpixelRemoval)
	, m_fxaaColorQueueH(DXGI_FORMAT_R11G11B10_FLOAT)
	, m_fxaaColorQueueV(DXGI_FORMAT_R11G11B10_FLOAT)
{}


void FXAA::Initialize(uint32_t width, uint32_t height)
{
	auto waitTask = concurrency::create_task([] {});

	m_resolveWorkCs.SetComputeShaderPath("Engine\\FXAAResolveWorkQueueCS", waitTask);

	if(DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{ 
		m_pass1HdrCs.SetComputeShaderPath("Engine\\FXAAPass1_Luma2_CS", waitTask);
		m_pass1LdrCs.SetComputeShaderPath("Engine\\FXAAPass1_RGB2_CS", waitTask);
		m_pass2HCs.SetComputeShaderPath("Engine\\FXAAPass2H2CS", waitTask);
		m_pass2VCs.SetComputeShaderPath("Engine\\FXAAPass2V2CS", waitTask);
	}
	else
	{
		m_pass1HdrCs.SetComputeShaderPath("Engine\\FXAAPass1_Luma_CS", waitTask);
		m_pass1LdrCs.SetComputeShaderPath("Engine\\FXAAPass1_RGB_CS", waitTask);
		m_pass2HCs.SetComputeShaderPath("Engine\\FXAAPass2HCS", waitTask);
		m_pass2VCs.SetComputeShaderPath("Engine\\FXAAPass2VCS", waitTask);
	}

	m_pass2HDebugCs.SetComputeShaderPath("Engine\\FXAAPass2HDebugCS", waitTask);
	m_pass2VDebugCs.SetComputeShaderPath("Engine\\FXAAPass2VDebugCS", waitTask);
	
	m_fxaaWorkQueueH.Create("FXAA Horizontal Work Queue", 512 * 1024, 4);
	m_fxaaWorkQueueV.Create("FXAA Vertical Work Queue", 512 * 1024, 4);
	m_fxaaColorQueueH.Create("FXAA Horizontal Color Queue", 512 * 1024, 4);
	m_fxaaColorQueueV.Create("FXAA Vertical Color Queue", 512 * 1024, 4);

	__declspec(align(16)) const uint32_t initArgs[6] = { 0, 1, 1, 0, 1, 1 };
	m_indirectParameters.Create("FXAA Indirect Parameters", 2, 3 * sizeof(uint32_t), initArgs);

	waitTask.wait();
}


void FXAA::Render(GraphicsCommandList& commandList)
{
	commandList.PIXBeginEvent("FXAA");

	auto target = DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT() ? m_sceneColorBuffer : m_postEffectsBuffer;

	{
		commandList.PIXBeginEvent("Pass 1");

		commandList.ResetCounter(m_fxaaWorkQueueH);
		commandList.ResetCounter(m_fxaaWorkQueueV);

		commandList.TransitionResource(*target, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_fxaaWorkQueueH, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_fxaaWorkQueueV, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_fxaaColorQueueH, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_fxaaColorQueueV, ResourceState::UnorderedAccess);

		auto& computeKernel = m_usePreComputedLuma ? m_pass1HdrCs : m_pass1LdrCs;

		computeKernel.GetResource("HWork")->SetUAVImmediate(m_fxaaWorkQueueH);
		computeKernel.GetResource("VWork")->SetUAVImmediate(m_fxaaWorkQueueV);
		computeKernel.GetResource("HColor")->SetUAVImmediate(m_fxaaColorQueueH);
		computeKernel.GetResource("VColor")->SetUAVImmediate(m_fxaaColorQueueV);
		computeKernel.GetResource("Color")->SetSRVImmediate(*target);

		computeKernel.GetParameter("RcpTextureSize")->SetValueImmediate(XMFLOAT2(1.0f / target->GetWidth(), 1.0f / target->GetHeight()));
		computeKernel.GetParameter("ContrastThreshold")->SetValueImmediate(m_contrastThreshold);
		computeKernel.GetParameter("SubpixelRemoval")->SetValueImmediate(m_subpixelRemoval);

		if (m_usePreComputedLuma)
		{
			commandList.TransitionResource(*m_lumaBuffer, ResourceState::NonPixelShaderResource);
			computeKernel.GetResource("Luma")->SetSRVImmediate(*m_lumaBuffer);
		}
		else
		{
			commandList.TransitionResource(*m_lumaBuffer, ResourceState::UnorderedAccess);
			computeKernel.GetResource("Luma")->SetUAVImmediate(*m_lumaBuffer);
		}

		auto& compCommandList = commandList.GetComputeCommandList();
		computeKernel.Dispatch2D(compCommandList, target->GetWidth(), target->GetHeight());
		computeKernel.UnbindUAVs(compCommandList);

		commandList.PIXEndEvent();
	}

	{
		commandList.PIXBeginEvent("Pass 2");

		// The next phase involves converting the work queues to DispatchIndirect parameters.
		// The queues are also padded out to 64 elements to simplify the final consume logic.

		commandList.TransitionResource(m_indirectParameters, ResourceState::UnorderedAccess);

#if DX11
		// This preserves the structured buffers' hidden counter values from the previous phase
		m_fxaaWorkQueueH.SetCounterInitialValue((uint32_t)-1);
		m_fxaaWorkQueueV.SetCounterInitialValue((uint32_t)-1);
#endif

		m_resolveWorkCs.GetResource("IndirectParams")->SetUAVImmediate(m_indirectParameters);
		m_resolveWorkCs.GetResource("WorkQueueH")->SetUAVImmediate(m_fxaaWorkQueueH);
		m_resolveWorkCs.GetResource("WorkQueueV")->SetUAVImmediate(m_fxaaWorkQueueV);

#if DX12
		// Read the structured buffers' counter buffers directly
		commandList.TransitionResource(*m_fxaaWorkQueueH.GetCounterBuffer(), ResourceState::GenericRead);
		commandList.TransitionResource(*m_fxaaWorkQueueV.GetCounterBuffer(), ResourceState::GenericRead);
#elif DX11
		commandList.CopyCounter(*m_fxaaWorkQueueH.GetCounterBuffer(), 0, m_fxaaWorkQueueH);
		commandList.CopyCounter(*m_fxaaWorkQueueV.GetCounterBuffer(), 0, m_fxaaWorkQueueV);
#endif
		m_resolveWorkCs.GetResource("WorkCounterH")->SetSRVImmediate(*m_fxaaWorkQueueH.GetCounterBuffer());
		m_resolveWorkCs.GetResource("WorkCounterV")->SetSRVImmediate(*m_fxaaWorkQueueV.GetCounterBuffer());

		auto& compCommandList = commandList.GetComputeCommandList();
		m_resolveWorkCs.Dispatch(compCommandList, 1, 1, 1);
		m_resolveWorkCs.UnbindUAVs(compCommandList);

		commandList.TransitionResource(*target, ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_indirectParameters, ResourceState::IndirectArgument);
		commandList.TransitionResource(m_fxaaWorkQueueH, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_fxaaWorkQueueV, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_fxaaColorQueueH, ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_fxaaColorQueueV, ResourceState::NonPixelShaderResource);

		// Horizontal pass
		{
			auto& computeKernel = m_debugDraw ? m_pass2HDebugCs : m_pass2HCs;

			computeKernel.GetParameter("RcpTextureSize")->SetValueImmediate(XMFLOAT2(1.0f / target->GetWidth(), 1.0f / target->GetHeight()));
			computeKernel.GetResource("DstColor")->SetUAVImmediate(*target);
			computeKernel.GetResource("Luma")->SetSRVImmediate(*m_lumaBuffer);
			computeKernel.GetResource("WorkQueue")->SetSRVImmediate(m_fxaaWorkQueueH);
			computeKernel.GetResource("ColorQueue")->SetSRVImmediate(m_fxaaColorQueueH);

			computeKernel.DispatchIndirect(compCommandList, m_indirectParameters, 0);
			computeKernel.UnbindUAVs(compCommandList);
		}


		// Vertical pass
		{
			auto& computeKernel = m_debugDraw ? m_pass2VDebugCs : m_pass2VCs;

			computeKernel.GetParameter("RcpTextureSize")->SetValueImmediate(XMFLOAT2(1.0f / target->GetWidth(), 1.0f / target->GetHeight()));
			computeKernel.GetResource("DstColor")->SetUAVImmediate(*target);
			computeKernel.GetResource("Luma")->SetSRVImmediate(*m_lumaBuffer);
			computeKernel.GetResource("WorkQueue")->SetSRVImmediate(m_fxaaWorkQueueV);
			computeKernel.GetResource("ColorQueue")->SetSRVImmediate(m_fxaaColorQueueV);

			computeKernel.DispatchIndirect(compCommandList, m_indirectParameters, 12);
			computeKernel.UnbindUAVs(compCommandList);
		}

		compCommandList.InsertUAVBarrier(*target);

		commandList.PIXEndEvent();
	}

	commandList.PIXEndEvent();
}