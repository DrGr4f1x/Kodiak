// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from SSAO.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "SSAO.h"

#include "Camera.h"
#include "ColorBuffer.h"
#include "CommandList.h"
#include "ComputeKernel.h"
#include "ComputeParameter.h"
#include "ComputeResource.h"
#include "DepthBuffer.h"
#include "Renderer.h"
#include "RenderTask.h"


using namespace Kodiak;
using namespace std;


SSAO::SSAO()
	: Enable(m_enabled)
	, DebugDraw(m_debugDraw)
	, ComputeLinearDepth(m_computeLinearZ)
	, SsaoFullscreen(m_ssaoFullscreen)
	, SceneColorBuffer(m_sceneColorBuffer)
	, SceneDepthBuffer(m_sceneDepthBuffer)
	, LinearDepthBuffer(m_linearDepth)
{}


void SSAO::Initialize()
{
	m_sampleThickness[0]	= sqrt(1.0f - 0.2f * 0.2f);
	m_sampleThickness[1]	= sqrt(1.0f - 0.4f * 0.4f);
	m_sampleThickness[2]	= sqrt(1.0f - 0.6f * 0.6f);
	m_sampleThickness[3]	= sqrt(1.0f - 0.8f * 0.8f);
	m_sampleThickness[4]	= sqrt(1.0f - 0.2f * 0.2f - 0.2f * 0.2f);
	m_sampleThickness[5]	= sqrt(1.0f - 0.2f * 0.2f - 0.4f * 0.4f);
	m_sampleThickness[6]	= sqrt(1.0f - 0.2f * 0.2f - 0.6f * 0.6f);
	m_sampleThickness[7]	= sqrt(1.0f - 0.2f * 0.2f - 0.8f * 0.8f);
	m_sampleThickness[8]	= sqrt(1.0f - 0.4f * 0.4f - 0.4f * 0.4f);
	m_sampleThickness[9]	= sqrt(1.0f - 0.4f * 0.4f - 0.6f * 0.6f);
	m_sampleThickness[10]	= sqrt(1.0f - 0.4f * 0.4f - 0.8f * 0.8f);
	m_sampleThickness[11]	= sqrt(1.0f - 0.6f * 0.6f - 0.6f * 0.6f);

	m_depthPrepare1Cs = make_shared<ComputeKernel>("depthPrepare1Cs");
	m_depthPrepare1Cs->SetComputeShaderPath("Engine", "AoPrepareDepthBuffers1CS.cso");

	m_depthPrepare2Cs = make_shared<ComputeKernel>("depthPrepare2Cs");
	m_depthPrepare2Cs->SetComputeShaderPath("Engine", "AoPrepareDepthBuffers2CS.cso");

	m_render1Cs = make_shared<ComputeKernel>("render1Cs");
	m_render1Cs->SetComputeShaderPath("Engine", "AoRender1CS.cso");

	m_render2Cs = make_shared<ComputeKernel>("render2Cs");
	m_render2Cs->SetComputeShaderPath("Engine", "AoRender2CS.cso");

	m_blurUpsampleBlend[0] = make_shared<ComputeKernel>("blurUpsampleBlendOutCs");
	m_blurUpsampleBlend[0]->SetComputeShaderPath("Engine", "AoBlurUpsampleBlendOutCS.cso");

	m_blurUpsampleBlend[1] = make_shared<ComputeKernel>("blurUpsamplePreMinBlendOutCs");
	m_blurUpsampleBlend[1]->SetComputeShaderPath("Engine", "AoBlurUpsamplePreMinBlendOutCS.cso");

	m_blurUpsampleFinal[0] = make_shared<ComputeKernel>("blurUpsampleCs");
	m_blurUpsampleFinal[0]->SetComputeShaderPath("Engine", "AoBlurUpsampleCS.cso");

	m_blurUpsampleFinal[1] = make_shared<ComputeKernel>("blurUpsamplePreMinCs");
	m_blurUpsampleFinal[1]->SetComputeShaderPath("Engine", "AoBlurUpsamplePreMinCS.cso");

	m_linearizeDepthCs = make_shared<ComputeKernel>("linearizeDepthCs");
	m_linearizeDepthCs->SetComputeShaderPath("Engine", "LinearizeDepthCS.cso");

	m_debugSsaoCs = make_shared<ComputeKernel>("debugSSAO");
	m_debugSsaoCs->SetComputeShaderPath("Engine", "DebugSSAOCS.cso");
}


void SSAO::SetCamera(shared_ptr<Camera> camera)
{
	m_camera = camera->GetRenderThreadData();
}


void SSAO::Render(GraphicsCommandList* commandList)
{
	const float zMagic = (m_camera->GetZFar() - m_camera->GetZNear()) / m_camera->GetZNear();

	if (!m_enabled)
	{
		commandList->PIXBeginEvent("Generate SSAO");

		commandList->ClearColor(*m_ssaoFullscreen);
		commandList->TransitionResource(*m_ssaoFullscreen, ResourceState::PixelShaderResource);

		if (!m_computeLinearZ)
		{
			commandList->PIXEndEvent();
			return;
		}

		auto computeCommandList = commandList->GetComputeCommandList();

		computeCommandList->TransitionResource(*m_sceneDepthBuffer, ResourceState::NonPixelShaderResource);

		m_linearizeDepthCs->GetParameter("ZMagic")->SetValue(zMagic);
		m_linearizeDepthCs->GetResource("Depth")->SetSRV(m_sceneDepthBuffer);
		m_linearizeDepthCs->GetResource("LinearZ")->SetUAV(m_linearDepth);
		
		m_linearizeDepthCs->Dispatch2D(computeCommandList, m_linearDepth->GetWidth(), m_linearDepth->GetHeight(), 16, 16);
		m_linearizeDepthCs->UnbindUAVs(computeCommandList);

		if (m_debugDraw)
		{
			computeCommandList->PIXBeginEvent("Debug draw");

			computeCommandList->TransitionResource(*m_sceneColorBuffer, ResourceState::UnorderedAccess);
			computeCommandList->TransitionResource(*m_linearDepth, ResourceState::NonPixelShaderResource);

			m_debugSsaoCs->GetResource("SsaoBuffer")->SetSRV(m_linearDepth);
			m_debugSsaoCs->GetResource("OutColor")->SetUAV(m_sceneColorBuffer);

			m_debugSsaoCs->Dispatch2D(computeCommandList, m_ssaoFullscreen->GetWidth(), m_ssaoFullscreen->GetHeight());
			m_debugSsaoCs->UnbindUAVs(computeCommandList);

			computeCommandList->PIXEndEvent();
		}

		commandList->PIXEndEvent();
		return;
	}
}