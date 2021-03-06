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

#include "CommandList.h"
#include "ComputeParameter.h"
#include "ComputeResource.h"
#include "DeviceManager.h"
#include "Format.h"
#include "PipelineState.h"
#include "RenderEnums.h"
#include "RenderUtils.h"
#include "Texture.h"


using namespace Kodiak;
using namespace std;
using namespace DirectX;


PostProcessing::PostProcessing()
	: SceneColorBuffer(m_sceneColorBuffer)
	, PostEffectsBuffer(m_postEffectsBuffer)
	, LumaBuffer(m_lumaBuffer)
	, EnableHDR(m_enableHDR)
	, DebugDrawSSAO(m_debugDrawSSAO)
	, EnableBloom(m_enableBloom)
	, EnableAdaptation(m_enableAdaptation)
	, HighQualityBloom(m_highQualityBloom)
	, DrawHistogram(m_drawHistogram)
	, DebugLuminance(m_debugLuminance)
	, BloomThreshold(m_bloomThreshold)
	, Exposure(m_exposure)
	, BloomUpsampleFactor(m_bloomUpsampleFactor)
	, BloomStrength(m_bloomStrength)
	, LogLumaConstant(m_logLumaConstant)
	, TargetLuminance(m_targetLuminance)
	, AdaptationRate(m_adaptationRate)
	, MinExposure(m_minExposure)
	, MaxExposure(m_maxExposure)
	, ToeStrength(m_toeStrength)
{}


void PostProcessing::Initialize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	const bool immediate = true;
	
	m_bloomExtractAndDownsampleHdrCs.SetComputeShaderPath("Engine\\BloomExtractAndDownsampleHdrCS.dx.cso", immediate);
	m_bloomExtractAndDownsampleLdrCs.SetComputeShaderPath("Engine\\BloomExtractAndDownsampleLdrCS.dx.cso", immediate);
	
	m_extractLumaCs.SetComputeShaderPath("Engine\\ExtractLumaCS.dx.cso", immediate);
	
	m_downsampleBloom4Cs.SetComputeShaderPath("Engine\\DownsampleBloomAllCS.dx.cso", immediate);
	
	m_downsampleBloom2Cs.SetComputeShaderPath("Engine\\DownsampleBloomCS.dx.cso", immediate);
	
	for (uint32_t i = 0; i < 5; ++i)
	{
		m_blurCs[i].SetComputeShaderPath("Engine\\BlurCS.dx.cso", immediate);
		m_upsampleAndBlurCs[i].SetComputeShaderPath("Engine\\UpsampleAndBlurCS.dx.cso", immediate);
	}

	if (DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{
		m_toneMapCs.SetComputeShaderPath("Engine\\ToneMap2CS.dx.cso", immediate);
		m_toneMapHdrCs.SetComputeShaderPath("Engine\\ToneMapHdr2CS.dx.cso", immediate);
		m_debugLuminanceHdrCs.SetComputeShaderPath("Engine\\DebugLuminanceHdr2CS.dx.cso", immediate);
		m_debugLuminanceLdrCs.SetComputeShaderPath("Engine\\DebugLuminanceLdr2CS.dx.cso", immediate);
	}
	else
	{
		m_toneMapCs.SetComputeShaderPath("Engine\\ToneMapCS.dx.cso", immediate);
		m_toneMapHdrCs.SetComputeShaderPath("Engine\\ToneMapHdrCS.dx.cso", immediate);
		m_debugLuminanceHdrCs.SetComputeShaderPath("Engine\\DebugLuminanceHdrCS.dx.cso", immediate);
		m_debugLuminanceLdrCs.SetComputeShaderPath("Engine\\DebugLuminanceLdrCS.dx.cso", immediate);
	}

	m_generateHistogramCs.SetComputeShaderPath("Engine\\GenerateHistogramCS.dx.cso", immediate);
	m_adaptExposureCs.SetComputeShaderPath("Engine\\AdaptExposureCS.dx.cso", immediate);
	m_debugDrawHistogramCs.SetComputeShaderPath("Engine\\DebugDrawHistogramCS.dx.cso", immediate);
	m_copyPostToSceneCs.SetComputeShaderPath("Engine\\CopyBackPostBufferCS.dx.cso", immediate);
	
	m_bloomUAV1[1].Create("Bloom Buffer", m_bloomWidth, m_bloomHeight, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV1[0].Create("Bloom Buffer 1a", m_bloomWidth, m_bloomHeight, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV2[0].Create("Bloom Buffer 2a", m_bloomWidth/2, m_bloomHeight/2, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV2[1].Create("Bloom Buffer 2b", m_bloomWidth/2, m_bloomHeight/2, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV3[0].Create("Bloom Buffer 3a", m_bloomWidth/4, m_bloomHeight/4, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV3[1].Create("Bloom Buffer 3b", m_bloomWidth/4, m_bloomHeight/4, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV4[0].Create("Bloom Buffer 4a", m_bloomWidth/8, m_bloomHeight/8, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV4[1].Create("Bloom Buffer 4b", m_bloomWidth/8, m_bloomHeight/8, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV5[0].Create("Bloom Buffer 5a", m_bloomWidth/16, m_bloomHeight/16, 1, ColorFormat::R11G11B10_Float);
	m_bloomUAV5[1].Create("Bloom Buffer 5b", m_bloomWidth/16, m_bloomHeight/16, 1, ColorFormat::R11G11B10_Float);

	m_lumaLR.Create("Luma Buffer", m_bloomWidth, m_bloomHeight, 1, ColorFormat::R8_UInt);

	__declspec(align(16)) float initExposure[] =
	{
		m_exposure, 1.0f / m_exposure, m_exposure, 0.0f,
		m_initialMinLog, m_initialMaxLog, m_initialMaxLog - m_initialMinLog, 1.0f / (m_initialMaxLog - m_initialMinLog)
	};
	m_exposureBuffer.Create("Exposure", 8, 4, initExposure);

	m_histogram.Create("Histogram", 256, 4);

	m_defaultBlackTexture = make_shared<Texture>();
	uint32_t blackPixel = 0;
	m_defaultBlackTexture->Create(1, 1, ColorFormat::R8G8B8A8, &blackPixel);

#if DX11
	InitializeSamplers();
#endif
}


void PostProcessing::Render(GraphicsCommandList& commandList)
{
	auto& compCommandList = commandList.GetComputeCommandList();

	compCommandList.PIXBeginEvent("Post Effects");

	compCommandList.TransitionResource(*m_sceneColorBuffer, ResourceState::ShaderResourceGeneric);
#if DX11
	ColorBuffer nullRenderTarget;
	commandList.SetRenderTarget(nullRenderTarget);
#endif

	if(m_enableHDR && !m_debugDrawSSAO)
	{ 
		ProcessHDR(compCommandList);
	}
	else
	{
		ProcessLDR(compCommandList);
	}

	

	commandList.PIXEndEvent();
}


void PostProcessing::FinalizePostProcessing(GraphicsCommandList& commandList)
{
	// In the case where we've been doing post processing in a separate buffer, we need to copy it
	// back to the original buffer.  It is possible to skip this step if the next shader knows to
	// do the manual format decode from UINT, but there are several code paths that need to be
	// changed, and some of them rely on texture filtering, which won't work with UINT.  Since this
	// is only to support legacy hardware and a single buffer copy isn't that big of a deal, this
	// is the most economical solution.

	if (!DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{
		auto& compCommandList = commandList.GetComputeCommandList();

		compCommandList.PIXBeginEvent("Copy Post back to Scene");

		compCommandList.TransitionResource(*m_sceneColorBuffer, ResourceState::UnorderedAccess);
		compCommandList.TransitionResource(*m_postEffectsBuffer, ResourceState::NonPixelShaderResource);

		m_copyPostToSceneCs.GetResource("SceneColor")->SetUAVImmediate(*m_sceneColorBuffer);
		m_copyPostToSceneCs.GetResource("PostBuffer")->SetSRVImmediate(*m_postEffectsBuffer);

		m_copyPostToSceneCs.Dispatch2D(compCommandList, m_sceneColorBuffer->GetWidth(), m_sceneColorBuffer->GetHeight());
		m_copyPostToSceneCs.UnbindUAVs(compCommandList);

		compCommandList.PIXEndEvent();
	}
}


void PostProcessing::RenderHistogram(GraphicsCommandList& commandList)
{
	if (m_drawHistogram)
	{
		auto& compCommandList = commandList.GetComputeCommandList();

		compCommandList.PIXBeginEvent("Debug Histogram");

		compCommandList.InsertUAVBarrier(*m_sceneColorBuffer);
		compCommandList.TransitionResource(m_histogram, ResourceState::NonPixelShaderResource);
		compCommandList.TransitionResource(m_exposureBuffer, ResourceState::NonPixelShaderResource);

		m_debugDrawHistogramCs.GetResource("Histogram")->SetSRVImmediate(m_histogram);
		m_debugDrawHistogramCs.GetResource("Exposure")->SetSRVImmediate(m_exposureBuffer);
		m_debugDrawHistogramCs.GetResource("ColorBuffer")->SetUAVImmediate(*m_sceneColorBuffer);

		m_debugDrawHistogramCs.Dispatch(compCommandList, 1, 32);

		compCommandList.TransitionResource(*m_sceneColorBuffer, ResourceState::NonPixelShaderResource);
		m_debugDrawHistogramCs.UnbindUAVs(compCommandList);

		compCommandList.PIXEndEvent();
	}
}


void PostProcessing::ProcessHDR(ComputeCommandList& commandList)
{
	commandList.PIXBeginEvent("HDR Tone Mapping");

	if (m_enableBloom)
	{
		GenerateBloom(commandList);
		commandList.TransitionResource(m_bloomUAV1[1], ResourceState::NonPixelShaderResource);
	}
	else if (m_enableAdaptation)
	{
		ExtractLuma(commandList);
	}

	if (DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{
		commandList.TransitionResource(*m_sceneColorBuffer, ResourceState::UnorderedAccess);
	}
	else
	{
		commandList.TransitionResource(*m_postEffectsBuffer, ResourceState::UnorderedAccess);
	}
	commandList.TransitionResource(*m_lumaBuffer, ResourceState::UnorderedAccess);
	commandList.TransitionResource(m_exposureBuffer, ResourceState::NonPixelShaderResource);

	auto& computeKernel = m_debugLuminance ? m_debugLuminanceHdrCs : (false ? m_toneMapHdrCs : m_toneMapCs);

	float toeStrength = m_toeStrength < 1e-6f ? 1e32f : 1.0f / m_toeStrength;

	// Set constants
	computeKernel.GetParameter("g_RcpBufferDim")->SetValueImmediate(XMFLOAT2(1.0f / m_sceneColorBuffer->GetWidth(), 1.0f / m_sceneColorBuffer->GetHeight()));
	computeKernel.GetParameter("g_BloomStrength")->SetValueImmediate(m_bloomStrength);
	computeKernel.GetParameter("g_ToeStrength")->SetValueImmediate(toeStrength);

	// Separate out LDR result from its perceived luminance
	if (DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{
		computeKernel.GetResource("ColorRW")->SetUAVImmediate(*m_sceneColorBuffer);
	}
	else
	{
		computeKernel.GetResource("DstColor")->SetUAVImmediate(*m_postEffectsBuffer);
		computeKernel.GetResource("SrcColor")->SetSRVImmediate(*m_sceneColorBuffer);
	}
	computeKernel.GetResource("OutLuma")->SetUAVImmediate(*m_lumaBuffer);

	// Read in original HDR value and blurred bloom buffer
	computeKernel.GetResource("Exposure")->SetSRVImmediate(m_exposureBuffer);
	if (m_enableBloom)
	{
		computeKernel.GetResource("Bloom")->SetSRVImmediate(m_bloomUAV1[1]);
	}
	else
	{
		computeKernel.GetResource("Bloom")->SetSRVImmediate(*m_defaultBlackTexture);
	}
	
	computeKernel.Dispatch2D(commandList, m_sceneColorBuffer->GetWidth(), m_sceneColorBuffer->GetHeight());
	computeKernel.UnbindUAVs(commandList);
	computeKernel.UnbindSRVs(commandList);

	// Do this last so that the bright pass uses the same exposure as tone mapping
	UpdateExposure(commandList);

	commandList.PIXEndEvent();
}


void PostProcessing::ProcessLDR(ComputeCommandList& commandList)
{
	commandList.PIXBeginEvent("LDR Processing");

	// TODO: implement this

	commandList.PIXEndEvent();
}


void PostProcessing::GenerateBloom(ComputeCommandList& commandList)
{
	commandList.PIXBeginEvent("Generate Bloom");

	auto& computeKernel = m_enableHDR ? m_bloomExtractAndDownsampleHdrCs : m_bloomExtractAndDownsampleLdrCs;

	const float invBloomWidth = 1.0f / static_cast<float>(m_bloomWidth);
	const float invBloomHeight = 1.0f / static_cast<float>(m_bloomHeight);

	computeKernel.GetParameter("g_inverseOutputSize")->SetValueImmediate(XMFLOAT2(invBloomWidth, invBloomHeight));
	computeKernel.GetParameter("g_bloomThreshold")->SetValueImmediate(m_bloomThreshold);

	commandList.TransitionResource(m_bloomUAV1[0], ResourceState::UnorderedAccess);
	commandList.TransitionResource(m_lumaLR, ResourceState::UnorderedAccess);
	commandList.TransitionResource(m_exposureBuffer, ResourceState::NonPixelShaderResource);

	computeKernel.GetResource("SourceTex")->SetSRVImmediate(*m_sceneColorBuffer);
	computeKernel.GetResource("Exposure")->SetSRVImmediate(m_exposureBuffer);
	computeKernel.GetResource("BloomResult")->SetUAVImmediate(m_bloomUAV1[0]);
	computeKernel.GetResource("LumaResult")->SetUAVImmediate(m_lumaLR);

	computeKernel.Dispatch2D(commandList, m_bloomWidth, m_bloomHeight);
	computeKernel.UnbindUAVs(commandList);
	computeKernel.UnbindSRVs(commandList);

	commandList.TransitionResource(m_bloomUAV1[0], ResourceState::NonPixelShaderResource);

	if (m_highQualityBloom)
	{
		commandList.TransitionResource(m_bloomUAV2[0], ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_bloomUAV3[0], ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_bloomUAV4[0], ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_bloomUAV5[0], ResourceState::UnorderedAccess);

		m_downsampleBloom4Cs.GetParameter("g_inverseDimensions")->SetValueImmediate(XMFLOAT2(invBloomWidth, invBloomHeight));
		m_downsampleBloom4Cs.GetResource("BloomBuf")->SetSRVImmediate(m_bloomUAV1[0]);
		m_downsampleBloom4Cs.GetResource("Result1")->SetUAVImmediate(m_bloomUAV2[0]);
		m_downsampleBloom4Cs.GetResource("Result2")->SetUAVImmediate(m_bloomUAV3[0]);
		m_downsampleBloom4Cs.GetResource("Result3")->SetUAVImmediate(m_bloomUAV4[0]);
		m_downsampleBloom4Cs.GetResource("Result4")->SetUAVImmediate(m_bloomUAV5[0]);

		m_downsampleBloom4Cs.Dispatch2D(commandList, m_bloomWidth / 2, m_bloomHeight / 2);
		m_downsampleBloom4Cs.UnbindUAVs(commandList);

		commandList.TransitionResource(m_bloomUAV2[0], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_bloomUAV3[0], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_bloomUAV4[0], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_bloomUAV5[0], ResourceState::NonPixelShaderResource);

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
		commandList.TransitionResource(m_bloomUAV3[0], ResourceState::UnorderedAccess);
		commandList.TransitionResource(m_bloomUAV5[0], ResourceState::UnorderedAccess);

		m_downsampleBloom2Cs.GetParameter("g_inverseDimensions")->SetValueImmediate(XMFLOAT2(invBloomWidth, invBloomHeight));
		m_downsampleBloom2Cs.GetResource("BloomBuf")->SetSRVImmediate(m_bloomUAV1[0]);
		m_downsampleBloom2Cs.GetResource("Result1")->SetUAVImmediate(m_bloomUAV3[0]);
		m_downsampleBloom2Cs.GetResource("Result2")->SetUAVImmediate(m_bloomUAV5[0]);

		m_downsampleBloom2Cs.Dispatch2D(commandList, m_bloomWidth / 2, m_bloomHeight / 2);
		m_downsampleBloom2Cs.UnbindUAVs(commandList);

		commandList.TransitionResource(m_bloomUAV3[0], ResourceState::NonPixelShaderResource);
		commandList.TransitionResource(m_bloomUAV5[0], ResourceState::NonPixelShaderResource);

		float upsampleBlendFactor = m_bloomUpsampleFactor * 2.0f / 3.0f;

		// Blur then upsample and blur two times
		BlurBuffer(commandList, 0, m_bloomUAV5, m_bloomUAV5[0], m_bloomWidth / 16, m_bloomHeight / 16, 1.0f);
		BlurBuffer(commandList, 1, m_bloomUAV3, m_bloomUAV5[1], m_bloomWidth / 4, m_bloomHeight / 4, upsampleBlendFactor);
		BlurBuffer(commandList, 2, m_bloomUAV1, m_bloomUAV3[1], m_bloomWidth, m_bloomHeight, upsampleBlendFactor);
	}

	commandList.PIXEndEvent();
}


void PostProcessing::ExtractLuma(ComputeCommandList& commandList)
{
	commandList.TransitionResource(m_lumaLR, ResourceState::UnorderedAccess);

	const float invBloomWidth = 1.0f / static_cast<float>(m_bloomWidth);
	const float invBloomHeight = 1.0f / static_cast<float>(m_bloomHeight);

	m_extractLumaCs.GetParameter("g_inverseOutputSize")->SetValueImmediate(XMFLOAT2(invBloomWidth, invBloomHeight));
	m_extractLumaCs.GetResource("SourceTex")->SetSRVImmediate(*m_sceneColorBuffer);
	m_extractLumaCs.GetResource("LumaResult")->SetUAVImmediate(m_lumaLR);

	m_extractLumaCs.Dispatch2D(commandList, m_bloomWidth, m_bloomHeight);
	m_extractLumaCs.UnbindUAVs(commandList);
}


void PostProcessing::BlurBuffer(ComputeCommandList& commandList, uint32_t blurKernelIndex, ColorBuffer buffer[2],
	ColorBuffer& lowerResBuf, uint32_t bufferWidth, uint32_t bufferHeight, float upsampleBlendFactor)
{
	// Select compute kernel: upsample-and-blur, or just blur
	const bool blurOnly = (&buffer[0] == &lowerResBuf);
	auto& computeKernel = blurOnly ? m_blurCs[blurKernelIndex] : m_upsampleAndBlurCs[blurKernelIndex];

	commandList.TransitionResource(buffer[1], ResourceState::UnorderedAccess);

	computeKernel.GetParameter("g_inverseDimensions")->SetValueImmediate(XMFLOAT2(1.0f / bufferWidth, 1.0f / bufferHeight));
	if (blurOnly)
	{
		computeKernel.GetResource("InputBuf")->SetSRVImmediate(buffer[0]);
		computeKernel.GetResource("Result")->SetUAVImmediate(buffer[1]);
	}
	else
	{
		computeKernel.GetParameter("g_upsampleBlendFactor")->SetValueImmediate(upsampleBlendFactor);
		computeKernel.GetResource("HigherResBuf")->SetSRVImmediate(buffer[0]);
		computeKernel.GetResource("LowerResBuf")->SetSRVImmediate(lowerResBuf);
		computeKernel.GetResource("Result")->SetUAVImmediate(buffer[1]);
	}

	computeKernel.Dispatch2D(commandList, bufferWidth, bufferHeight);
	computeKernel.UnbindUAVs(commandList);

	commandList.TransitionResource(buffer[1], ResourceState::NonPixelShaderResource);
}


void PostProcessing::UpdateExposure(ComputeCommandList& commandList)
{
	commandList.PIXBeginEvent("Update Exposure");

	if (!m_enableAdaptation)
	{
		__declspec(align(16)) float initExposure[] =
		{
			m_exposure, 1.0f / m_exposure, m_exposure, 0.0f,
			m_initialMinLog, m_initialMaxLog, m_initialMaxLog - m_initialMinLog, 1.0f / (m_initialMaxLog - m_initialMinLog)
		};

		commandList.WriteBuffer(m_exposureBuffer, 0, initExposure, sizeof(initExposure));
		commandList.TransitionResource(m_exposureBuffer, ResourceState::NonPixelShaderResource);

		commandList.PIXEndEvent();

		return;
	}

	commandList.ClearUAV(m_histogram);
	commandList.TransitionResource(m_histogram, ResourceState::UnorderedAccess);
	commandList.TransitionResource(m_lumaLR, ResourceState::NonPixelShaderResource);

	m_generateHistogramCs.GetResource("LumaBuf")->SetSRVImmediate(m_lumaLR);
	m_generateHistogramCs.GetResource("Histogram")->SetUAVImmediate(m_histogram);

	m_generateHistogramCs.Dispatch2D(commandList, m_bloomWidth, m_bloomHeight, 16, 384);
	m_generateHistogramCs.UnbindUAVs(commandList);

	commandList.TransitionResource(m_histogram, ResourceState::NonPixelShaderResource);
	commandList.TransitionResource(m_exposureBuffer, ResourceState::UnorderedAccess);

	m_adaptExposureCs.GetResource("Histogram")->SetSRVImmediate(m_histogram);
	m_adaptExposureCs.GetResource("Exposure")->SetUAVImmediate(m_exposureBuffer);

	m_adaptExposureCs.GetParameter("TargetLuminance")->SetValueImmediate(m_targetLuminance);
	m_adaptExposureCs.GetParameter("AdaptationRate")->SetValueImmediate(m_adaptationRate);
	m_adaptExposureCs.GetParameter("MinExposure")->SetValueImmediate(m_minExposure);
	m_adaptExposureCs.GetParameter("MaxExposure")->SetValueImmediate(m_maxExposure);
	m_adaptExposureCs.GetParameter("PixelCount")->SetValueImmediate(static_cast<float>(m_bloomWidth * m_bloomHeight));

	m_adaptExposureCs.Dispatch(commandList);
	m_adaptExposureCs.UnbindUAVs(commandList);

	commandList.TransitionResource(m_exposureBuffer, ResourceState::NonPixelShaderResource);

	commandList.PIXEndEvent();
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