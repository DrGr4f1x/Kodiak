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
#include "Format.h"
#include "Renderer.h"
#include "RenderTask.h"


using namespace Kodiak;
using namespace std;
using namespace DirectX;


SSAO::SSAO()
	: Enable(m_enabled)
	, DebugDraw(m_debugDraw)
	, ComputeLinearDepth(m_computeLinearZ)
	, SsaoFullscreen(m_ssaoFullscreen)
	, SceneColorBuffer(m_sceneColorBuffer)
	, SceneDepthBuffer(m_sceneDepthBuffer)
	, LinearDepthBuffer(m_linearDepth)
{}


void SSAO::Initialize(uint32_t width, uint32_t height)
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

	const uint32_t bufferWidth1 = (width + 1) / 2;
	const uint32_t bufferWidth2 = (width + 3) / 4;
	const uint32_t bufferWidth3 = (width + 7) / 8;
	const uint32_t bufferWidth4 = (width + 15) / 16;
	const uint32_t bufferWidth5 = (width + 31) / 32;
	const uint32_t bufferWidth6 = (width + 63) / 64;
	const uint32_t bufferHeight1 = (height + 1) / 2;
	const uint32_t bufferHeight2 = (height + 3) / 4;
	const uint32_t bufferHeight3 = (height + 7) / 8;
	const uint32_t bufferHeight4 = (height + 15) / 16;
	const uint32_t bufferHeight5 = (height + 31) / 32;
	const uint32_t bufferHeight6 = (height + 63) / 64;

	m_depthDownsize1 = make_shared<ColorBuffer>();
	m_depthDownsize2 = make_shared<ColorBuffer>();
	m_depthDownsize3 = make_shared<ColorBuffer>();
	m_depthDownsize4 = make_shared<ColorBuffer>();
	m_depthTiled1 = make_shared<ColorBuffer>();
	m_depthTiled2 = make_shared<ColorBuffer>();
	m_depthTiled3 = make_shared<ColorBuffer>();
	m_depthTiled4 = make_shared<ColorBuffer>();
	m_depthDownsize1->Create("Depth downsized 1", bufferWidth1, bufferHeight1, 1, ColorFormat::R32_Float);
	m_depthDownsize2->Create("Depth downsized 2", bufferWidth2, bufferHeight2, 1, ColorFormat::R32_Float);
	m_depthDownsize3->Create("Depth downsized 3", bufferWidth3, bufferHeight3, 1, ColorFormat::R32_Float);
	m_depthDownsize4->Create("Depth downsized 4", bufferWidth4, bufferHeight4, 1, ColorFormat::R32_Float);
	m_depthTiled1->CreateArray("Depth de-interleaved 1", bufferWidth3, bufferHeight3, 16, ColorFormat::R16_Float);
	m_depthTiled2->CreateArray("Depth de-interleaved 2", bufferWidth4, bufferHeight4, 16, ColorFormat::R16_Float);
	m_depthTiled3->CreateArray("Depth de-interleaved 3", bufferWidth5, bufferHeight5, 16, ColorFormat::R16_Float);
	m_depthTiled4->CreateArray("Depth de-interleaved 4", bufferWidth6, bufferHeight6, 16, ColorFormat::R16_Float);
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

	commandList->PIXBeginEvent("Generate SSAO");

	commandList->TransitionResource(*m_sceneDepthBuffer, ResourceState::NonPixelShaderResource);
	commandList->TransitionResource(*m_ssaoFullscreen, ResourceState::UnorderedAccess);

	auto computeCommandList = commandList->GetComputeCommandList();

	// Decompress and downsample
	{
		computeCommandList->PIXBeginEvent("Decompress and downsample");

		m_depthPrepare1Cs->GetParameter("ZMagic")->SetValueImmediate(zMagic);
		m_depthPrepare1Cs->GetResource("Depth")->SetSRVImmediate(m_sceneDepthBuffer);

		computeCommandList->TransitionResource(*m_linearDepth, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_depthDownsize1, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_depthTiled1, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_depthDownsize2, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_depthTiled2, ResourceState::UnorderedAccess);

		m_depthPrepare1Cs->GetResource("LinearZ")->SetUAVImmediate(m_linearDepth);
		m_depthPrepare1Cs->GetResource("DS2x")->SetUAVImmediate(m_depthDownsize1);
		m_depthPrepare1Cs->GetResource("DS2Atlas")->SetUAVImmediate(m_depthTiled1);
		m_depthPrepare1Cs->GetResource("DS4x")->SetUAVImmediate(m_depthDownsize2);
		m_depthPrepare1Cs->GetResource("DS4Atlas")->SetUAVImmediate(m_depthTiled2);

		m_depthPrepare1Cs->Dispatch2D(computeCommandList, m_depthTiled2->GetWidth() * 8, m_depthTiled2->GetHeight() * 8);
		m_depthPrepare1Cs->UnbindUAVs(computeCommandList);

		if (m_hierarchyDepth > 2)
		{
			XMFLOAT2 invSize(1.0f / m_depthDownsize2->GetWidth(), 1.0f / m_depthDownsize2->GetHeight());
			m_depthPrepare2Cs->GetParameter("InvSourceDimension")->SetValueImmediate(invSize);

			computeCommandList->TransitionResource(*m_depthDownsize2, ResourceState::NonPixelShaderResource);
			computeCommandList->TransitionResource(*m_depthDownsize3, ResourceState::UnorderedAccess);
			computeCommandList->TransitionResource(*m_depthTiled3, ResourceState::UnorderedAccess);
			computeCommandList->TransitionResource(*m_depthDownsize4, ResourceState::UnorderedAccess);
			computeCommandList->TransitionResource(*m_depthTiled4, ResourceState::UnorderedAccess);

			m_depthPrepare2Cs->GetResource("DS4x")->SetSRVImmediate(m_depthDownsize2);
			m_depthPrepare2Cs->GetResource("DS8x")->SetUAVImmediate(m_depthDownsize3);
			m_depthPrepare2Cs->GetResource("DS8xAtlas")->SetUAVImmediate(m_depthTiled3);
			m_depthPrepare2Cs->GetResource("DS16x")->SetUAVImmediate(m_depthDownsize4);
			m_depthPrepare2Cs->GetResource("DS16xAtlas")->SetUAVImmediate(m_depthTiled4);

			m_depthPrepare2Cs->Dispatch2D(computeCommandList, m_depthTiled4->GetWidth() * 8, m_depthTiled4->GetHeight() * 8);
			m_depthPrepare2Cs->UnbindUAVs(computeCommandList);
		}

		computeCommandList->PIXEndEvent();
	}

	// Analyze depth volumes
	if(false)
	{
		const auto& projMat = m_camera->GetProjectionMatrix();
		const float fovTangent = 1.0f / (projMat.GetX().GetX());

		computeCommandList->TransitionResource(*m_aoMerged1, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_aoMerged2, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_aoMerged3, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_aoMerged4, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_aoHighQuality1, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_aoHighQuality2, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_aoHighQuality3, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_aoHighQuality4, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_depthTiled1, ResourceState::NonPixelShaderResource);
		computeCommandList->TransitionResource(*m_depthTiled2, ResourceState::NonPixelShaderResource);
		computeCommandList->TransitionResource(*m_depthTiled3, ResourceState::NonPixelShaderResource);
		computeCommandList->TransitionResource(*m_depthTiled4, ResourceState::NonPixelShaderResource);
		computeCommandList->TransitionResource(*m_depthDownsize1, ResourceState::NonPixelShaderResource);
		computeCommandList->TransitionResource(*m_depthDownsize2, ResourceState::NonPixelShaderResource);
		computeCommandList->TransitionResource(*m_depthDownsize3, ResourceState::NonPixelShaderResource);
		computeCommandList->TransitionResource(*m_depthDownsize4, ResourceState::NonPixelShaderResource);
	}

	commandList->PIXEndEvent();
}