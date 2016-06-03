// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from PostEffects.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "PostProcessing.h"

#include "ColorBuffer.h"
#include "CommandList.h"
#include "ComputeKernel.h"
#include "ComputeParameter.h"
#include "ComputeResource.h"
#include "Format.h"
#include "GpuBuffer.h"
#include "RenderEnums.h"

using namespace Kodiak;
using namespace std;
using namespace DirectX;


PostProcessing::PostProcessing()
	: SceneColorBuffer(m_sceneColorBuffer)
	, EnableHDR(m_enableHDR)
	, DebugDrawSSAO(m_debugDrawSSAO)
	, EnableBloom(m_enableBloom)
	, EnableAdaptation(m_enableAdaptation)
	, BloomThreshold(m_bloomThreshold)
	, Exposure(m_exposure)
	, PeakIntensity(m_peakIntensity)
{}


void PostProcessing::Initialize(uint32_t width, uint32_t height)
{
	m_bloomExtractAndDownsampleHdrCs = make_shared<ComputeKernel>();
	m_bloomExtractAndDownsampleHdrCs->SetComputeShaderPath("Engine", "BloomExtractAndDownsampleHdrCS.cso");
	auto waitTask = m_bloomExtractAndDownsampleHdrCs->loadTask;

	m_bloomExtractAndDownsampleLdrCs = make_shared<ComputeKernel>();
	m_bloomExtractAndDownsampleLdrCs->SetComputeShaderPath("Engine", "BloomExtractAndDownsampleLdrCS.cso");
	waitTask = waitTask && m_bloomExtractAndDownsampleLdrCs->loadTask;

	for (uint32_t i = 0; i < 2; ++i)
	{
		m_bloomUAV1[i] = make_shared<ColorBuffer>();
		m_bloomUAV2[i] = make_shared<ColorBuffer>();
		m_bloomUAV3[i] = make_shared<ColorBuffer>();
		m_bloomUAV4[i] = make_shared<ColorBuffer>();
		m_bloomUAV5[i] = make_shared<ColorBuffer>();
	}

	m_bloomUAV1[1]->Create("Bloom Buffer", m_bloomWidth, m_bloomHeight, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV1[0]->Create("Bloom Buffer 1a", m_bloomWidth, m_bloomHeight, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV2[0]->Create("Bloom Buffer 2a", m_bloomWidth/2, m_bloomHeight/2, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV2[1]->Create("Bloom Buffer 2b", m_bloomWidth/2, m_bloomHeight/2, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV3[0]->Create("Bloom Buffer 3a", m_bloomWidth/4, m_bloomHeight/4, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV3[1]->Create("Bloom Buffer 3b", m_bloomWidth/4, m_bloomHeight/4, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV4[0]->Create("Bloom Buffer 4a", m_bloomWidth/8, m_bloomHeight/8, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV4[1]->Create("Bloom Buffer 4b", m_bloomWidth/8, m_bloomHeight/8, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV5[0]->Create("Bloom Buffer 5a", m_bloomWidth/16, m_bloomHeight/16, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV5[1]->Create("Bloom Buffer 5b", m_bloomWidth/16, m_bloomHeight/16, 1, ColorFormat::R11G11B10_Float);

	m_lumaBuffer = make_shared<ColorBuffer>();
	m_lumaBuffer->Create("Luminance", width, height, 1, ColorFormat::R8_UNorm);

	m_lumaLR = make_shared<ColorBuffer>();
	m_lumaLR->Create("Luma Buffer", m_bloomWidth, m_bloomHeight, 1, ColorFormat::R8_UNorm);

	__declspec(align(16)) float initExposure[] =
	{
		m_exposure, 1.0f / m_exposure, m_exposure / m_peakIntensity, 0.0f,
		m_initialMinLog, m_initialMaxLog, m_initialMaxLog - m_initialMinLog, 1.0f / (m_initialMaxLog - m_initialMinLog)
	};
	m_exposureBuffer = make_shared<StructuredBuffer>();
	m_exposureBuffer->Create("Exposure", 8, 4, initExposure);

	waitTask.wait();
}


void PostProcessing::Render(GraphicsCommandList* commandList)
{
	ComputeCommandList* computeCommandList = commandList->GetComputeCommandList();

	computeCommandList->PIXBeginEvent("Post Effects");

	computeCommandList->TransitionResource(*m_sceneColorBuffer, ResourceState::NonPixelShaderResource);

	if(m_enableHDR && !m_debugDrawSSAO)
	{ 
		ProcessHDR(computeCommandList);
	}
	else
	{
		ProcessLDR(computeCommandList);
	}


	commandList->PIXEndEvent();
}


void PostProcessing::ProcessHDR(ComputeCommandList* commandList)
{
	commandList->PIXBeginEvent("HDR Tone Mapping");

	if (m_enableBloom)
	{
		GenerateBloom(commandList);
		commandList->TransitionResource(*m_bloomUAV1[1], ResourceState::NonPixelShaderResource);
	}
	else if (m_enableAdaptation)
	{
		//ExtractLuma(commandList);
	}

	commandList->TransitionResource(*m_sceneColorBuffer, ResourceState::UnorderedAccess);
	commandList->TransitionResource(*m_lumaBuffer, ResourceState::UnorderedAccess);
	commandList->TransitionResource(*m_exposureBuffer, ResourceState::NonPixelShaderResource);



	commandList->PIXEndEvent();
}


void PostProcessing::ProcessLDR(ComputeCommandList* commandList)
{
	commandList->PIXBeginEvent("LDR Processing");

	commandList->PIXEndEvent();
}


void PostProcessing::GenerateBloom(ComputeCommandList* commandList)
{
	commandList->PIXBeginEvent("Generate Bloom");

	auto computeKernel = m_enableHDR ? m_bloomExtractAndDownsampleHdrCs : m_bloomExtractAndDownsampleLdrCs;

	const float invBloomWidth = 1.0f / static_cast<float>(m_bloomWidth);
	const float invBloomHeight = 1.0f / static_cast<float>(m_bloomHeight);

	computeKernel->GetParameter("g_inverseOutputSize")->SetValueImmediate(XMFLOAT2(invBloomWidth, invBloomHeight));
	computeKernel->GetParameter("g_bloomThreshold")->SetValueImmediate(m_bloomThreshold);

	commandList->TransitionResource(*m_bloomUAV1[0], ResourceState::UnorderedAccess);
	commandList->TransitionResource(*m_lumaLR, ResourceState::UnorderedAccess);
	commandList->TransitionResource(*m_sceneColorBuffer, ResourceState::NonPixelShaderResource);
	commandList->TransitionResource(*m_exposureBuffer, ResourceState::NonPixelShaderResource);

	computeKernel->GetResource("SourceTex")->SetSRV(m_sceneColorBuffer);
	//computeKernel->GetResource("Exposure")->SetSRV(m_exposureBuffer);
	computeKernel->GetResource("BloomResult")->SetUAV(m_bloomUAV1[0]);
	computeKernel->GetResource("LumaResult")->SetUAV(m_lumaLR);

	commandList->PIXEndEvent();
}