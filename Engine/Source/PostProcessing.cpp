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
#include "DeviceManager.h"
#include "Format.h"
#include "GpuBuffer.h"
#include "PipelineState.h"
#include "ShaderManager.h"
#include "RenderEnums.h"
#include "Texture.h"


using namespace Kodiak;
using namespace std;
using namespace DirectX;


PostProcessing::PostProcessing()
	: SceneColorBuffer(m_sceneColorBuffer)
	, EnableHDR(m_enableHDR)
	, DebugDrawSSAO(m_debugDrawSSAO)
	, EnableBloom(m_enableBloom)
	, EnableAdaptation(m_enableAdaptation)
	, HighQualityBloom(m_highQualityBloom)
	, ToneMapOnlyLuma(m_toneMapOnlyLuma)
	, BloomThreshold(m_bloomThreshold)
	, Exposure(m_exposure)
	, PeakIntensity(m_peakIntensity)
	, BloomUpsampleFactor(m_bloomUpsampleFactor)
	, BloomStrength(m_bloomStrength)
	, LogLumaConstant(m_logLumaConstant)
	, TargetLuminance(m_targetLuminance)
	, AdaptationRate(m_adaptationRate)
	, MinExposure(m_minExposure)
	, MaxExposure(m_maxExposure)
{}


void PostProcessing::Initialize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	m_bloomExtractAndDownsampleHdrCs = make_shared<ComputeKernel>();
	m_bloomExtractAndDownsampleHdrCs->SetComputeShaderPath("Engine", "BloomExtractAndDownsampleHdrCS.cso");
	auto waitTask = m_bloomExtractAndDownsampleHdrCs->loadTask;

	m_bloomExtractAndDownsampleLdrCs = make_shared<ComputeKernel>();
	m_bloomExtractAndDownsampleLdrCs->SetComputeShaderPath("Engine", "BloomExtractAndDownsampleLdrCS.cso");
	waitTask = waitTask && m_bloomExtractAndDownsampleLdrCs->loadTask;

	m_extractLumaCs = make_shared<ComputeKernel>();
	m_extractLumaCs->SetComputeShaderPath("Engine", "ExtractLumaCS.cso");
	waitTask = waitTask && m_extractLumaCs->loadTask;

	m_downsampleBloom4Cs = make_shared<ComputeKernel>();
	m_downsampleBloom4Cs->SetComputeShaderPath("Engine", "DownsampleBloom4CS.cso");
	waitTask = waitTask && m_downsampleBloom4Cs->loadTask;

	m_downsampleBloom2Cs = make_shared<ComputeKernel>();
	m_downsampleBloom2Cs->SetComputeShaderPath("Engine", "DownsampleBloom2CS.cso");
	waitTask = waitTask && m_downsampleBloom2Cs->loadTask;

	for (uint32_t i = 0; i < 5; ++i)
	{
		m_blurCs[i] = make_shared<ComputeKernel>();
		m_blurCs[i]->SetComputeShaderPath("Engine", "BlurCS.cso");
		waitTask = waitTask && m_blurCs[i]->loadTask;

		m_upsampleAndBlurCs[i] = make_shared<ComputeKernel>();
		m_upsampleAndBlurCs[i]->SetComputeShaderPath("Engine", "UpsampleAndBlurCS.cso");
		waitTask = waitTask && m_upsampleAndBlurCs[i]->loadTask;
	}

	m_toneMapCs = make_shared<ComputeKernel>();
	m_toneMapCs->SetComputeShaderPath("Engine", "ToneMapCS.cso");
	waitTask = waitTask && m_toneMapCs->loadTask;

	m_toneMap2Cs = make_shared<ComputeKernel>();
	m_toneMap2Cs->SetComputeShaderPath("Engine", "ToneMap2CS.cso");
	waitTask = waitTask && m_toneMap2Cs->loadTask;

	m_generateHistogramCs = make_shared<ComputeKernel>();
	m_generateHistogramCs->SetComputeShaderPath("Engine", "GenerateHistogramCS.cso");
	waitTask = waitTask && m_generateHistogramCs->loadTask;

	m_adaptExposureCs = make_shared<ComputeKernel>();
	m_adaptExposureCs->SetComputeShaderPath("Engine", "AdaptExposureCS.cso");
	waitTask = waitTask && m_adaptExposureCs->loadTask;

	m_debugDrawHistogramCs = make_shared<ComputeKernel>();
	m_debugDrawHistogramCs->SetComputeShaderPath("Engine", "DebugDrawHistogramCS.cso");
	waitTask = waitTask && m_debugDrawHistogramCs->loadTask;

	if (m_copySceneColor)
	{
		m_copyPSO = make_shared<GraphicsPSO>();

		BlendStateDesc defaultBlendState;
		DepthStencilStateDesc depthStencilState(false, false);
		RasterizerStateDesc rasterizerState(CullMode::None, FillMode::Solid);

		auto vs = ShaderManager::GetInstance().LoadVertexShader(ShaderPath("Engine", "ScreenQuadVS.cso"));
		auto ps = ShaderManager::GetInstance().LoadPixelShader(ShaderPath("Engine", "BufferCopyPS.cso"));
		(vs->loadTask && ps->loadTask).wait();

		m_copyPSO->SetBlendState(defaultBlendState);
		m_copyPSO->SetRasterizerState(rasterizerState);
		m_copyPSO->SetDepthStencilState(depthStencilState);
		m_copyPSO->SetVertexShader(vs.get());
		m_copyPSO->SetPixelShader(ps.get());

		m_copyPSO->Finalize();
	}

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
	m_lumaLR->Create("Luma Buffer", m_bloomWidth, m_bloomHeight, 1, ColorFormat::R8_UInt);

	__declspec(align(16)) float initExposure[] =
	{
		m_exposure, 1.0f / m_exposure, m_exposure / m_peakIntensity, 0.0f,
		m_initialMinLog, m_initialMaxLog, m_initialMaxLog - m_initialMinLog, 1.0f / (m_initialMaxLog - m_initialMinLog)
	};
	m_exposureBuffer = make_shared<StructuredBuffer>();
	m_exposureBuffer->Create("Exposure", 8, 4, initExposure);

	m_histogram = make_shared<ByteAddressBuffer>();
	m_histogram->Create("Histogram", 256, 4);

	m_defaultBlackTexture = make_shared<Texture>();
	uint32_t blackPixel = 0;
	m_defaultBlackTexture->Create(1, 1, ColorFormat::R8G8B8A8, &blackPixel);

#if DX11
	if (m_copySceneColor)
	{
		m_sceneColorCopy = make_shared<ColorBuffer>();
		m_sceneColorCopy->Create("Scene color copy", width, height, 1, ColorFormat::R11G11B10_Float);
	}

	InitializeSamplers();
#endif

	waitTask.wait();
}


void PostProcessing::Render(GraphicsCommandList* commandList)
{
	ComputeCommandList* computeCommandList = commandList->GetComputeCommandList();

	computeCommandList->PIXBeginEvent("Post Effects");

	computeCommandList->TransitionResource(*m_sceneColorBuffer, ResourceState::PixelShaderResource);

#if DX11
	ColorBuffer nullBuffer;
	commandList->SetRenderTarget(nullBuffer);

	if (m_copySceneColor)
	{
		commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetPipelineState(*m_copyPSO);

		// Copy and convert the LDR present source to the current back buffer
		commandList->SetRenderTarget(*m_sceneColorCopy);
		commandList->SetPixelShaderResource(0, m_sceneColorBuffer->GetSRV());

		commandList->SetViewportAndScissor(0, 0, m_width, m_height);

		commandList->Draw(3);

		commandList->SetPixelShaderResource(0, nullptr);
		commandList->SetRenderTarget(nullBuffer);
	}

	computeCommandList->SetShaderSampler(0, m_linearClampSampler.Get());
	computeCommandList->SetShaderSampler(1, m_linearBorderSampler.Get());
#endif

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


void PostProcessing::DebugDrawHistogram(GraphicsCommandList* commandList)
{
	ComputeCommandList* computeCommandList = commandList->GetComputeCommandList();

	computeCommandList->PIXBeginEvent("Debug Histogram");

	computeCommandList->InsertUAVBarrier(*m_sceneColorBuffer);
	computeCommandList->TransitionResource(*m_histogram, ResourceState::NonPixelShaderResource);
	computeCommandList->TransitionResource(*m_exposureBuffer, ResourceState::NonPixelShaderResource);

	m_debugDrawHistogramCs->GetResource("Histogram")->SetSRVImmediate(m_histogram);
	m_debugDrawHistogramCs->GetResource("Exposure")->SetSRVImmediate(m_exposureBuffer);
	m_debugDrawHistogramCs->GetResource("ColorBuffer")->SetUAVImmediate(m_sceneColorBuffer);

	m_debugDrawHistogramCs->Dispatch(computeCommandList, 1, 32);

	computeCommandList->TransitionResource(*m_sceneColorBuffer, ResourceState::NonPixelShaderResource);
	m_debugDrawHistogramCs->UnbindUAVs(computeCommandList);

	computeCommandList->PIXEndEvent();
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
		ExtractLuma(commandList);
	}

	commandList->TransitionResource(*m_sceneColorBuffer, ResourceState::UnorderedAccess);
	commandList->TransitionResource(*m_lumaBuffer, ResourceState::UnorderedAccess);
	commandList->TransitionResource(*m_exposureBuffer, ResourceState::NonPixelShaderResource);

	auto computeKernel = m_toneMapOnlyLuma ? m_toneMap2Cs : m_toneMapCs;

	computeKernel->GetParameter("g_RcpBufferDim")->SetValueImmediate(XMFLOAT2(1.0f / m_sceneColorBuffer->GetWidth(), 1.0f / m_sceneColorBuffer->GetHeight()));
	computeKernel->GetParameter("g_BloomStrength")->SetValueImmediate(m_bloomStrength);
	computeKernel->GetParameter("g_LumaGamma")->SetValueImmediate(m_logLumaConstant);

#if DX11
	computeKernel->GetResource("SrcColor")->SetSRVImmediate(m_sceneColorCopy);
#else
	computeKernel->GetResource("SrcColor")->SetSRVImmediate(m_sceneColorBuffer);
#endif
	computeKernel->GetResource("Exposure")->SetSRVImmediate(m_exposureBuffer);
	if (m_enableBloom)
	{
		computeKernel->GetResource("Bloom")->SetSRVImmediate(m_bloomUAV1[1]);
	}
	else
	{
		computeKernel->GetResource("Bloom")->SetSRVImmediate(m_defaultBlackTexture);
	}

	computeKernel->GetResource("DstColor")->SetUAVImmediate(m_sceneColorBuffer);
	computeKernel->GetResource("OutLuma")->SetUAVImmediate(m_lumaBuffer);

	computeKernel->Dispatch2D(commandList, m_sceneColorBuffer->GetWidth(), m_sceneColorBuffer->GetHeight());
	computeKernel->UnbindUAVs(commandList);
	computeKernel->UnbindSRVs(commandList);

	// Do this last so that the bright pass uses the same exposure as tone mapping
	UpdateExposure(commandList);

	commandList->PIXEndEvent();
}


void PostProcessing::ProcessLDR(ComputeCommandList* commandList)
{
	commandList->PIXBeginEvent("LDR Processing");

	// TODO: implement this

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
#if DX11
	commandList->TransitionResource(*m_sceneColorCopy, ResourceState::NonPixelShaderResource);
#else
	commandList->TransitionResource(*m_sceneColorBuffer, ResourceState::NonPixelShaderResource);
#endif
	commandList->TransitionResource(*m_exposureBuffer, ResourceState::NonPixelShaderResource);

#if DX11
	computeKernel->GetResource("SourceTex")->SetSRVImmediate(m_sceneColorCopy);
#else
	computeKernel->GetResource("SourceTex")->SetSRVImmediate(m_sceneColorBuffer);
#endif
	computeKernel->GetResource("Exposure")->SetSRVImmediate(m_exposureBuffer);
	computeKernel->GetResource("BloomResult")->SetUAVImmediate(m_bloomUAV1[0]);
	computeKernel->GetResource("LumaResult")->SetUAVImmediate(m_lumaLR);

	if (m_enableHDR)
	{
		m_bloomExtractAndDownsampleHdrCs->Dispatch2D(commandList, m_bloomWidth, m_bloomHeight);
		m_bloomExtractAndDownsampleHdrCs->UnbindUAVs(commandList);
		m_bloomExtractAndDownsampleHdrCs->UnbindSRVs(commandList);
	}
	else
	{
		m_bloomExtractAndDownsampleLdrCs->Dispatch2D(commandList, m_bloomWidth, m_bloomHeight);
		m_bloomExtractAndDownsampleLdrCs->UnbindUAVs(commandList);
		m_bloomExtractAndDownsampleLdrCs->UnbindSRVs(commandList);
	}

	commandList->TransitionResource(*m_bloomUAV1[0], ResourceState::NonPixelShaderResource);

	if (m_highQualityBloom)
	{
		commandList->TransitionResource(*m_bloomUAV2[0], ResourceState::UnorderedAccess);
		commandList->TransitionResource(*m_bloomUAV3[0], ResourceState::UnorderedAccess);
		commandList->TransitionResource(*m_bloomUAV4[0], ResourceState::UnorderedAccess);
		commandList->TransitionResource(*m_bloomUAV5[0], ResourceState::UnorderedAccess);

		m_downsampleBloom4Cs->GetParameter("g_inverseDimensions")->SetValueImmediate(XMFLOAT2(invBloomWidth, invBloomHeight));
		m_downsampleBloom4Cs->GetResource("BloomBuf")->SetSRVImmediate(m_bloomUAV1[0]);
		m_downsampleBloom4Cs->GetResource("Result1")->SetUAVImmediate(m_bloomUAV2[0]);
		m_downsampleBloom4Cs->GetResource("Result2")->SetUAVImmediate(m_bloomUAV3[0]);
		m_downsampleBloom4Cs->GetResource("Result3")->SetUAVImmediate(m_bloomUAV4[0]);
		m_downsampleBloom4Cs->GetResource("Result4")->SetUAVImmediate(m_bloomUAV5[0]);

		m_downsampleBloom4Cs->Dispatch2D(commandList, m_bloomWidth / 2, m_bloomHeight / 2);
		m_downsampleBloom4Cs->UnbindUAVs(commandList);

		commandList->TransitionResource(*m_bloomUAV2[0], ResourceState::NonPixelShaderResource);
		commandList->TransitionResource(*m_bloomUAV3[0], ResourceState::NonPixelShaderResource);
		commandList->TransitionResource(*m_bloomUAV4[0], ResourceState::NonPixelShaderResource);
		commandList->TransitionResource(*m_bloomUAV5[0], ResourceState::NonPixelShaderResource);

		float upsampleBlendFactor = m_bloomUpsampleFactor;

		// Blur then upsample and blur four times
		BlurBuffer(commandList, 0, m_bloomUAV5, m_bloomUAV5[0], m_bloomWidth / 16, m_bloomHeight / 16, 1.0f);
		BlurBuffer(commandList, 1, m_bloomUAV4, m_bloomUAV5[1], m_bloomWidth / 8, m_bloomHeight / 8, upsampleBlendFactor);
		BlurBuffer(commandList, 2, m_bloomUAV3, m_bloomUAV4[1], m_bloomWidth / 4, m_bloomHeight / 4, upsampleBlendFactor);
		BlurBuffer(commandList, 3, m_bloomUAV2, m_bloomUAV3[1], m_bloomWidth / 2, m_bloomHeight / 2, upsampleBlendFactor);
		BlurBuffer(commandList, 1, m_bloomUAV1, m_bloomUAV2[1], m_bloomWidth, m_bloomHeight, upsampleBlendFactor);
	}
	else
	{
		commandList->TransitionResource(*m_bloomUAV3[0], ResourceState::UnorderedAccess);
		commandList->TransitionResource(*m_bloomUAV5[0], ResourceState::UnorderedAccess);

		m_downsampleBloom2Cs->GetParameter("g_inverseDimensions")->SetValueImmediate(XMFLOAT2(invBloomWidth, invBloomHeight));
		m_downsampleBloom2Cs->GetResource("BloomBuf")->SetSRVImmediate(m_bloomUAV1[0]);
		m_downsampleBloom2Cs->GetResource("Result1")->SetUAVImmediate(m_bloomUAV3[0]);
		m_downsampleBloom2Cs->GetResource("Result2")->SetUAVImmediate(m_bloomUAV5[0]);

		m_downsampleBloom2Cs->Dispatch2D(commandList, m_bloomWidth / 2, m_bloomHeight / 2);
		m_downsampleBloom2Cs->UnbindUAVs(commandList);

		commandList->TransitionResource(*m_bloomUAV3[0], ResourceState::NonPixelShaderResource);
		commandList->TransitionResource(*m_bloomUAV5[0], ResourceState::NonPixelShaderResource);

		float upsampleBlendFactor = m_bloomUpsampleFactor * 2.0f / 3.0f;

		// Blur then upsample and blur two times
		BlurBuffer(commandList, 0, m_bloomUAV5, m_bloomUAV5[0], m_bloomWidth / 16, m_bloomHeight / 16, 1.0f);
		BlurBuffer(commandList, 1, m_bloomUAV3, m_bloomUAV5[1], m_bloomWidth / 4, m_bloomHeight / 4, upsampleBlendFactor);
		BlurBuffer(commandList, 2, m_bloomUAV1, m_bloomUAV3[1], m_bloomWidth, m_bloomHeight, upsampleBlendFactor);
	}

	commandList->PIXEndEvent();
}


void PostProcessing::ExtractLuma(ComputeCommandList* commandList)
{
	commandList->TransitionResource(*m_lumaLR, ResourceState::UnorderedAccess);

	const float invBloomWidth = 1.0f / static_cast<float>(m_bloomWidth);
	const float invBloomHeight = 1.0f / static_cast<float>(m_bloomHeight);

	m_extractLumaCs->GetParameter("g_inverseOutputSize")->SetValueImmediate(XMFLOAT2(invBloomWidth, invBloomHeight));
#if DX11
	m_extractLumaCs->GetResource("SourceTex")->SetSRVImmediate(m_sceneColorCopy);
#else
	m_extractLumaCs->GetResource("SourceTex")->SetSRVImmediate(m_sceneColorBuffer);
#endif
	m_extractLumaCs->GetResource("LumaResult")->SetUAVImmediate(m_lumaLR);

	m_extractLumaCs->Dispatch2D(commandList, m_bloomWidth, m_bloomHeight);
	m_extractLumaCs->UnbindUAVs(commandList);
}


void PostProcessing::BlurBuffer(ComputeCommandList* commandList, uint32_t blurKernelIndex, std::shared_ptr<ColorBuffer> buffer[2],
	std::shared_ptr<ColorBuffer> lowerResBuf, uint32_t bufferWidth, uint32_t bufferHeight, float upsampleBlendFactor)
{
	// Select compute kernel: upsample-and-blur, or just blur
	const bool blurOnly = (buffer[0] == lowerResBuf);
	auto computeKernel = blurOnly ? m_blurCs[blurKernelIndex] : m_upsampleAndBlurCs[blurKernelIndex];

	commandList->TransitionResource(*buffer[1], ResourceState::UnorderedAccess);

	computeKernel->GetParameter("g_inverseDimensions")->SetValueImmediate(XMFLOAT2(1.0f / bufferWidth, 1.0f / bufferHeight));
	if (blurOnly)
	{
		computeKernel->GetResource("InputBuf")->SetSRVImmediate(buffer[0]);
		computeKernel->GetResource("Result")->SetUAVImmediate(buffer[1]);
	}
	else
	{
		computeKernel->GetParameter("g_upsampleBlendFactor")->SetValueImmediate(upsampleBlendFactor);
		computeKernel->GetResource("HigherResBuf")->SetSRVImmediate(buffer[0]);
		computeKernel->GetResource("LowerResBuf")->SetSRVImmediate(lowerResBuf);
		computeKernel->GetResource("Result")->SetUAVImmediate(buffer[1]);
	}

	computeKernel->Dispatch2D(commandList, bufferWidth, bufferHeight);
	computeKernel->UnbindUAVs(commandList);

	commandList->TransitionResource(*buffer[1], ResourceState::NonPixelShaderResource);
}


void PostProcessing::UpdateExposure(ComputeCommandList* commandList)
{
	commandList->PIXBeginEvent("Update Exposure");

	if (!m_enableAdaptation)
	{
		__declspec(align(16)) float initExposure[] =
		{
			m_exposure, 1.0f / m_exposure, m_exposure / m_peakIntensity, 0.0f,
			m_initialMinLog, m_initialMaxLog, m_initialMaxLog - m_initialMinLog, 1.0f / (m_initialMaxLog - m_initialMinLog)
		};

		commandList->WriteBuffer(*m_exposureBuffer, 0, initExposure, sizeof(initExposure));
		commandList->TransitionResource(*m_exposureBuffer, ResourceState::NonPixelShaderResource);

		return;
	}

	commandList->ClearUAV(*m_histogram);
	commandList->TransitionResource(*m_histogram, ResourceState::UnorderedAccess);
	commandList->TransitionResource(*m_lumaLR, ResourceState::NonPixelShaderResource);

	m_generateHistogramCs->GetResource("LumaBuf")->SetSRVImmediate(m_lumaLR);
	m_generateHistogramCs->GetResource("Histogram")->SetUAVImmediate(m_histogram);

	m_generateHistogramCs->Dispatch2D(commandList, m_bloomWidth, m_bloomHeight, 16, 384);
	m_generateHistogramCs->UnbindUAVs(commandList);

	commandList->TransitionResource(*m_histogram, ResourceState::NonPixelShaderResource);
	commandList->TransitionResource(*m_exposureBuffer, ResourceState::UnorderedAccess);

	m_adaptExposureCs->GetResource("Histogram")->SetSRVImmediate(m_histogram);
	m_adaptExposureCs->GetResource("Exposure")->SetUAVImmediate(m_exposureBuffer);

	m_adaptExposureCs->GetParameter("TargetLuminance")->SetValueImmediate(m_targetLuminance);
	m_adaptExposureCs->GetParameter("AdaptationRate")->SetValueImmediate(m_adaptationRate);
	m_adaptExposureCs->GetParameter("MinExposure")->SetValueImmediate(m_minExposure);
	m_adaptExposureCs->GetParameter("MaxExposure")->SetValueImmediate(m_maxExposure);
	m_adaptExposureCs->GetParameter("PeakIntensity")->SetValueImmediate(m_peakIntensity);
	m_adaptExposureCs->GetParameter("PixelCount")->SetValueImmediate(static_cast<float>(m_bloomWidth * m_bloomHeight));

	m_adaptExposureCs->Dispatch(commandList);
	m_adaptExposureCs->UnbindUAVs(commandList);

	commandList->TransitionResource(*m_exposureBuffer, ResourceState::NonPixelShaderResource);

	commandList->PIXEndEvent();
}


#if DX11
void PostProcessing::InitializeSamplers()
{
	auto samplerDesc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	ThrowIfFailed(g_device->CreateSamplerState(&samplerDesc, m_linearBorderSampler.GetAddressOf()));

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ThrowIfFailed(g_device->CreateSamplerState(&samplerDesc, m_linearClampSampler.GetAddressOf()));
}
#endif