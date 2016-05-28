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
#include "DeviceManager.h"
#include "Format.h"
#include "Renderer.h"
#include "RenderTask.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace std;
using namespace DirectX;
using namespace Math;


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
	m_sampleThickness[0] = sqrt(1.0f - 0.2f * 0.2f);
	m_sampleThickness[1] = sqrt(1.0f - 0.4f * 0.4f);
	m_sampleThickness[2] = sqrt(1.0f - 0.6f * 0.6f);
	m_sampleThickness[3] = sqrt(1.0f - 0.8f * 0.8f);
	m_sampleThickness[4] = sqrt(1.0f - 0.2f * 0.2f - 0.2f * 0.2f);
	m_sampleThickness[5] = sqrt(1.0f - 0.2f * 0.2f - 0.4f * 0.4f);
	m_sampleThickness[6] = sqrt(1.0f - 0.2f * 0.2f - 0.6f * 0.6f);
	m_sampleThickness[7] = sqrt(1.0f - 0.2f * 0.2f - 0.8f * 0.8f);
	m_sampleThickness[8] = sqrt(1.0f - 0.4f * 0.4f - 0.4f * 0.4f);
	m_sampleThickness[9] = sqrt(1.0f - 0.4f * 0.4f - 0.6f * 0.6f);
	m_sampleThickness[10] = sqrt(1.0f - 0.4f * 0.4f - 0.8f * 0.8f);
	m_sampleThickness[11] = sqrt(1.0f - 0.6f * 0.6f - 0.6f * 0.6f);

	m_depthPrepare1Cs = make_shared<ComputeKernel>("depthPrepare1Cs");
	m_depthPrepare1Cs->SetComputeShaderPath("Engine", "AoPrepareDepthBuffers1CS.cso");
	auto waitTask = m_depthPrepare1Cs->loadTask;

	m_depthPrepare2Cs = make_shared<ComputeKernel>("depthPrepare2Cs");
	m_depthPrepare2Cs->SetComputeShaderPath("Engine", "AoPrepareDepthBuffers2CS.cso");
	waitTask = waitTask && m_depthPrepare2Cs->loadTask;

	for (int32_t i = 0; i < 4; ++i)
	{
		m_render1Cs[i] = make_shared<ComputeKernel>("render1Cs");
		m_render1Cs[i]->SetComputeShaderPath("Engine", "AoRender1CS.cso");
		waitTask = waitTask && m_render1Cs[i]->loadTask;

		m_render2Cs[i] = make_shared<ComputeKernel>("render2Cs");
		m_render2Cs[i]->SetComputeShaderPath("Engine", "AoRender2CS.cso");
		waitTask = waitTask && m_render2Cs[i]->loadTask;

		m_blurUpsampleBlend[i][0] = make_shared<ComputeKernel>("blurUpsampleBlendOutCs");
		m_blurUpsampleBlend[i][0]->SetComputeShaderPath("Engine", "AoBlurUpsampleBlendOutCS.cso");
		waitTask = waitTask && m_blurUpsampleBlend[i][0]->loadTask;

		m_blurUpsampleBlend[i][1] = make_shared<ComputeKernel>("blurUpsamplePreMinBlendOutCs");
		m_blurUpsampleBlend[i][1]->SetComputeShaderPath("Engine", "AoBlurUpsamplePreMinBlendOutCS.cso");
		waitTask = waitTask && m_blurUpsampleBlend[i][1]->loadTask;

		m_blurUpsampleFinal[i][0] = make_shared<ComputeKernel>("blurUpsampleCs");
		m_blurUpsampleFinal[i][0]->SetComputeShaderPath("Engine", "AoBlurUpsampleCS.cso");
		waitTask = waitTask && m_blurUpsampleFinal[i][0]->loadTask;

		m_blurUpsampleFinal[i][1] = make_shared<ComputeKernel>("blurUpsamplePreMinCs");
		m_blurUpsampleFinal[i][1]->SetComputeShaderPath("Engine", "AoBlurUpsamplePreMinCS.cso");
		waitTask = waitTask && m_blurUpsampleFinal[i][1]->loadTask;
	}

	m_linearizeDepthCs = make_shared<ComputeKernel>("linearizeDepthCs");
	m_linearizeDepthCs->SetComputeShaderPath("Engine", "LinearizeDepthCS.cso");
	waitTask = waitTask && m_linearizeDepthCs->loadTask;

	m_debugSsaoCs = make_shared<ComputeKernel>("debugSSAO");
	m_debugSsaoCs->SetComputeShaderPath("Engine", "DebugSSAOCS.cso");
	waitTask = waitTask && m_debugSsaoCs->loadTask;

	waitTask.wait();

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

	m_aoMerged1 = make_shared<ColorBuffer>();
	m_aoMerged2 = make_shared<ColorBuffer>();
	m_aoMerged3 = make_shared<ColorBuffer>();
	m_aoMerged4 = make_shared<ColorBuffer>();
	m_aoSmooth1 = make_shared<ColorBuffer>();
	m_aoSmooth2 = make_shared<ColorBuffer>();
	m_aoSmooth3 = make_shared<ColorBuffer>();
	m_aoHighQuality1 = make_shared<ColorBuffer>();
	m_aoHighQuality2 = make_shared<ColorBuffer>();
	m_aoHighQuality3 = make_shared<ColorBuffer>();
	m_aoHighQuality4 = make_shared<ColorBuffer>();
	m_aoMerged1->Create("AO re-interleaved 1", bufferWidth1, bufferHeight1, 1, ColorFormat::R8_UNorm);
	m_aoMerged2->Create("AO re-interleaved 2", bufferWidth2, bufferHeight2, 1, ColorFormat::R8_UNorm);
	m_aoMerged3->Create("AO re-interleaved 3", bufferWidth3, bufferHeight3, 1, ColorFormat::R8_UNorm);
	m_aoMerged4->Create("AO re-interleaved 4", bufferWidth4, bufferHeight4, 1, ColorFormat::R8_UNorm);
	m_aoSmooth1->Create("AO smoothed 1", bufferWidth1, bufferHeight1, 1, ColorFormat::R8_UNorm);
	m_aoSmooth2->Create("AO smoothed 2", bufferWidth2, bufferHeight2, 1, ColorFormat::R8_UNorm);
	m_aoSmooth3->Create("AO smoothed 3", bufferWidth3, bufferHeight3, 1, ColorFormat::R8_UNorm);
	m_aoHighQuality1->Create("AO high quality 1", bufferWidth1, bufferHeight1, 1, ColorFormat::R8_UNorm);
	m_aoHighQuality2->Create("AO high quality 2", bufferWidth2, bufferHeight2, 1, ColorFormat::R8_UNorm);
	m_aoHighQuality3->Create("AO high quality 3", bufferWidth3, bufferHeight3, 1, ColorFormat::R8_UNorm);
	m_aoHighQuality4->Create("AO high quality 4", bufferWidth4, bufferHeight4, 1, ColorFormat::R8_UNorm);

#if DX11
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
#endif
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
		m_linearizeDepthCs->UnbindSRVs(computeCommandList);
		m_linearizeDepthCs->UnbindUAVs(computeCommandList);

		if (m_debugDraw)
		{
			computeCommandList->PIXBeginEvent("Debug draw");

			computeCommandList->TransitionResource(*m_sceneColorBuffer, ResourceState::UnorderedAccess);
			computeCommandList->TransitionResource(*m_linearDepth, ResourceState::NonPixelShaderResource);

			m_debugSsaoCs->GetResource("SsaoBuffer")->SetSRV(m_linearDepth);
			m_debugSsaoCs->GetResource("OutColor")->SetUAV(m_sceneColorBuffer);

			m_debugSsaoCs->Dispatch2D(computeCommandList, m_ssaoFullscreen->GetWidth(), m_ssaoFullscreen->GetHeight());
			m_debugSsaoCs->UnbindSRVs(computeCommandList);
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
		m_depthPrepare1Cs->GetResource("DS2xAtlas")->SetUAVImmediate(m_depthTiled1);
		m_depthPrepare1Cs->GetResource("DS4x")->SetUAVImmediate(m_depthDownsize2);
		m_depthPrepare1Cs->GetResource("DS4xAtlas")->SetUAVImmediate(m_depthTiled2);

		m_depthPrepare1Cs->Dispatch2D(computeCommandList, m_depthTiled2->GetWidth() * 8, m_depthTiled2->GetHeight() * 8);
		m_depthPrepare1Cs->UnbindSRVs(computeCommandList);
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
			m_depthPrepare2Cs->UnbindSRVs(computeCommandList);
			m_depthPrepare2Cs->UnbindUAVs(computeCommandList);
		}

		computeCommandList->PIXEndEvent();
	}

	// Analyze depth volumes
	{
		const auto& projMat = m_camera->GetProjectionMatrix();
		const float fovTangent = 1.0f / (projMat.GetX().GetX());

		computeCommandList->PIXBeginEvent("Analyze depth volumes");

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

		// Render SSAO for each sub-tile
		if (m_hierarchyDepth > 3)
		{
			ComputeAO(computeCommandList, m_render1Cs[0], m_aoMerged4, m_depthTiled4, fovTangent);
			if (m_qualityLevel >= kSsaoQualityLow)
			{
				ComputeAO(computeCommandList, m_render2Cs[0], m_aoHighQuality4, m_depthDownsize4, fovTangent);
			}
		}
		if (m_hierarchyDepth > 2)
		{
			ComputeAO(computeCommandList, m_render1Cs[1], m_aoMerged3, m_depthTiled3, fovTangent);
			if (m_qualityLevel >= kSsaoQualityMedium)
			{
				ComputeAO(computeCommandList, m_render2Cs[1], m_aoHighQuality3, m_depthDownsize3, fovTangent);
			}
		}
		if (m_hierarchyDepth > 1)
		{
			ComputeAO(computeCommandList, m_render1Cs[2], m_aoMerged2, m_depthTiled2, fovTangent);
			if (m_qualityLevel >= kSsaoQualityHigh)
			{
				ComputeAO(computeCommandList, m_render2Cs[2], m_aoHighQuality2, m_depthDownsize2, fovTangent);
			}
		}
		{
			ComputeAO(computeCommandList, m_render1Cs[3], m_aoMerged1, m_depthTiled1, fovTangent);
			if (m_qualityLevel >= kSsaoQualityVeryHigh)
			{
				ComputeAO(computeCommandList, m_render2Cs[3], m_aoHighQuality1, m_depthDownsize1, fovTangent);
			}
		}

		computeCommandList->PIXEndEvent();
	}

	// Blur and upsample
	{
		computeCommandList->PIXBeginEvent("Blur and upsample");

		shared_ptr<ColorBuffer> nextSRV = m_aoMerged4;

		m_currentBlurUpsampleBlend = 0;
		m_currentBlurUpsampleFinal = 0;

		if (m_hierarchyDepth > 3)
		{
			BlurAndUpsample(computeCommandList, m_aoSmooth3, m_depthDownsize3, m_depthDownsize4, nextSRV,
				m_qualityLevel >= kSsaoQualityLow ? m_aoHighQuality4 : nullptr, m_aoMerged3);

			nextSRV = m_aoSmooth3;
		}
		else
		{
			nextSRV = m_aoMerged3;
		}

		if (m_hierarchyDepth > 2)
		{
			BlurAndUpsample(computeCommandList, m_aoSmooth2, m_depthDownsize2, m_depthDownsize3, nextSRV,
				m_qualityLevel >= kSsaoQualityMedium ? m_aoHighQuality3 : nullptr, m_aoMerged2);

			nextSRV = m_aoSmooth2;
		}
		else
		{
			nextSRV = m_aoMerged2;
		}

		if (m_hierarchyDepth > 1)
		{
			BlurAndUpsample(computeCommandList, m_aoSmooth1, m_depthDownsize1, m_depthDownsize2, nextSRV,
				m_qualityLevel >= kSsaoQualityHigh ? m_aoHighQuality2 : nullptr, m_aoMerged1);

			nextSRV = m_aoSmooth1;
		}
		else
		{
			nextSRV = m_aoMerged1;
		}

		BlurAndUpsample(computeCommandList, m_ssaoFullscreen, m_linearDepth, m_depthDownsize1, nextSRV,
			m_qualityLevel >= kSsaoQualityVeryHigh ? m_aoHighQuality1 : nullptr, nullptr);

		computeCommandList->PIXEndEvent();
	}

	if(m_debugDraw)
	{
		computeCommandList->PIXBeginEvent("Debug draw");

		computeCommandList->TransitionResource(*m_sceneColorBuffer, ResourceState::UnorderedAccess);
		computeCommandList->TransitionResource(*m_ssaoFullscreen, ResourceState::NonPixelShaderResource);

		m_debugSsaoCs->GetResource("SsaoBuffer")->SetSRVImmediate(m_ssaoFullscreen);
		m_debugSsaoCs->GetResource("OutColor")->SetUAVImmediate(m_sceneColorBuffer);

		m_debugSsaoCs->Dispatch2D(computeCommandList, m_ssaoFullscreen->GetWidth(), m_ssaoFullscreen->GetHeight());
		m_debugSsaoCs->UnbindSRVs(computeCommandList);
		m_debugSsaoCs->UnbindUAVs(computeCommandList);

		computeCommandList->PIXEndEvent();
	}

	commandList->PIXEndEvent();
}


void SSAO::ComputeAO(ComputeCommandList* commandList, shared_ptr<ComputeKernel> kernel, shared_ptr<ColorBuffer> destination,
	shared_ptr<ColorBuffer> depthBuffer, const float tanHalfFovH)
{
	uint32_t bufferWidth = depthBuffer->GetWidth();
	uint32_t bufferHeight = depthBuffer->GetHeight();
	uint32_t arrayCount = depthBuffer->GetArraySize();

	// Here we compute multipliers that convert the center depth value into (the reciprocal of)
	// sphere thicknesses at each sample location.  This assumes a maximum sample radius of 5
	// units, but since a sphere has no thickness at its extent, we don't need to sample that far
	// out.  Only samples whole integer offsets with distance less than 25 are used.  This means
	// that there is no sample at (3, 4) because its distance is exactly 25 (and has a thickness of 0.)

	// The shaders are set up to sample a circular region within a 5-pixel radius.
	const float screenspaceDiameter = 10.0f;

	// SphereDiameter = CenterDepth * ThicknessMultiplier.  This will compute the thickness of a sphere centered
	// at a specific depth.  The ellipsoid scale can stretch a sphere into an ellipsoid, which changes the
	// characteristics of the AO.
	// TanHalfFovH:  Radius of sphere in depth units if its center lies at Z = 1
	// ScreenspaceDiameter:  Diameter of sample sphere in pixel units
	// ScreenspaceDiameter / BufferWidth:  Ratio of the screen width that the sphere actually covers
	// Note about the "2.0f * ":  Diameter = 2 * Radius
	float thicknessMultiplier = 2.0f * tanHalfFovH * screenspaceDiameter / bufferWidth;

	if (arrayCount == 1)
	{
		thicknessMultiplier *= 2.0f;
	}

	// This will transform a depth value from [0, thickness] to [0, 1].
	float inverseRangeFactor = 1.0f / thicknessMultiplier;

	__declspec(align(16)) float ssaoCB[28];

	// The thicknesses are smaller for all off-center samples of the sphere.  Compute thicknesses relative
	// to the center sample.
	ssaoCB[0] = inverseRangeFactor / m_sampleThickness[0];
	ssaoCB[1] = inverseRangeFactor / m_sampleThickness[1];
	ssaoCB[2] = inverseRangeFactor / m_sampleThickness[2];
	ssaoCB[3] = inverseRangeFactor / m_sampleThickness[3];
	ssaoCB[4] = inverseRangeFactor / m_sampleThickness[4];
	ssaoCB[5] = inverseRangeFactor / m_sampleThickness[5];
	ssaoCB[6] = inverseRangeFactor / m_sampleThickness[6];
	ssaoCB[7] = inverseRangeFactor / m_sampleThickness[7];
	ssaoCB[8] = inverseRangeFactor / m_sampleThickness[8];
	ssaoCB[9] = inverseRangeFactor / m_sampleThickness[9];
	ssaoCB[10] = inverseRangeFactor / m_sampleThickness[10];
	ssaoCB[11] = inverseRangeFactor / m_sampleThickness[11];

	// These are the weights that are multiplied against the samples because not all samples are
	// equally important.  The farther the sample is from the center location, the less they matter.
	// We use the thickness of the sphere to determine the weight.  The scalars in front are the number
	// of samples with this weight because we sum the samples together before multiplying by the weight,
	// so as an aggregate all of those samples matter more.  After generating this table, the weights
	// are normalized.
	ssaoCB[12] = 4.0f * m_sampleThickness[0];		// Axial
	ssaoCB[13] = 4.0f * m_sampleThickness[1];		// Axial
	ssaoCB[14] = 4.0f * m_sampleThickness[2];		// Axial
	ssaoCB[15] = 4.0f * m_sampleThickness[3];		// Axial
	ssaoCB[16] = 4.0f * m_sampleThickness[4];		// Diagonal
	ssaoCB[17] = 8.0f * m_sampleThickness[5];		// L-shaped
	ssaoCB[18] = 8.0f * m_sampleThickness[6];		// L-shaped
	ssaoCB[19] = 8.0f * m_sampleThickness[7];		// L-shaped
	ssaoCB[20] = 4.0f * m_sampleThickness[8];		// Diagonal
	ssaoCB[21] = 8.0f * m_sampleThickness[9];		// L-shaped
	ssaoCB[22] = 8.0f * m_sampleThickness[10];	// L-shaped
	ssaoCB[23] = 4.0f * m_sampleThickness[11];	// Diagonal

//#define SAMPLE_EXHAUSTIVELY

// If we aren't using all of the samples, delete their weights before we normalize.
#ifndef SAMPLE_EXHAUSTIVELY
	ssaoCB[12] = 0.0f;
	ssaoCB[14] = 0.0f;
	ssaoCB[17] = 0.0f;
	ssaoCB[19] = 0.0f;
	ssaoCB[21] = 0.0f;
#endif

	// Normalize the weights by dividing by the sum of all weights
	float totalWeight = 0.0f;
	for (int i = 12; i < 24; ++i)
	{
		totalWeight += ssaoCB[i];
	}
	for (int i = 12; i < 24; ++i)
	{
		ssaoCB[i] /= totalWeight;
	}

	ssaoCB[24] = 1.0f / bufferWidth;
	ssaoCB[25] = 1.0f / bufferHeight;
	ssaoCB[26] = 1.0f / -m_rejectionFalloff;
	ssaoCB[27] = 1.0f / (1.0f + m_accentuation);

	kernel->GetParameter("gInvThicknessTable")->SetValueImmediate(Vector4(ssaoCB[0], ssaoCB[1], ssaoCB[2], ssaoCB[3]), 0);
	kernel->GetParameter("gInvThicknessTable")->SetValueImmediate(Vector4(ssaoCB[4], ssaoCB[5], ssaoCB[6], ssaoCB[7]), 1);
	kernel->GetParameter("gInvThicknessTable")->SetValueImmediate(Vector4(ssaoCB[8], ssaoCB[9], ssaoCB[10], ssaoCB[11]), 2);
	kernel->GetParameter("gSampleWeightTable")->SetValueImmediate(Vector4(ssaoCB[12], ssaoCB[13], ssaoCB[14], ssaoCB[15]), 0);
	kernel->GetParameter("gSampleWeightTable")->SetValueImmediate(Vector4(ssaoCB[16], ssaoCB[17], ssaoCB[18], ssaoCB[19]), 1);
	kernel->GetParameter("gSampleWeightTable")->SetValueImmediate(Vector4(ssaoCB[20], ssaoCB[21], ssaoCB[22], ssaoCB[23]), 2);
	kernel->GetParameter("gInvSliceDimension")->SetValueImmediate(XMFLOAT2(ssaoCB[24], ssaoCB[25]));
	kernel->GetParameter("gRejectFadeoff")->SetValueImmediate(ssaoCB[26]);
	kernel->GetParameter("gRcpAccentuation")->SetValueImmediate(ssaoCB[27]);

	kernel->GetResource("DepthTex")->SetSRVImmediate(depthBuffer);
	kernel->GetResource("Occlusion")->SetUAVImmediate(destination);

#if DX11
	commandList->SetShaderSampler(1, m_linearBorderSampler.Get());
#endif

	if (arrayCount == 1)
	{
		kernel->Dispatch2D(commandList, bufferWidth, bufferHeight, 16, 16);
	}
	else
	{
		kernel->Dispatch3D(commandList, bufferWidth, bufferHeight, arrayCount, 8, 8, 1);
	}
	kernel->UnbindSRVs(commandList);
	kernel->UnbindUAVs(commandList);
}


void SSAO::BlurAndUpsample(ComputeCommandList* commandList,
	shared_ptr<ColorBuffer> destination, shared_ptr<ColorBuffer> hiResDepth, shared_ptr<ColorBuffer> loResDepth,
	shared_ptr<ColorBuffer> interleavedAO, shared_ptr<ColorBuffer> highQualityAO, shared_ptr<ColorBuffer> hiResAO)
{
	uint32_t loWidth = loResDepth->GetWidth();
	uint32_t loHeight = loResDepth->GetHeight();
	uint32_t hiWidth = hiResDepth->GetWidth();
	uint32_t hiHeight = hiResDepth->GetHeight();

	shared_ptr<ComputeKernel> kernel = nullptr;
	if (hiResAO == nullptr)
	{
		kernel = m_blurUpsampleFinal[m_currentBlurUpsampleFinal][highQualityAO == nullptr ? 0 : 1];
		++m_currentBlurUpsampleFinal;
	}
	else
	{
		kernel = m_blurUpsampleBlend[m_currentBlurUpsampleBlend][highQualityAO == nullptr ? 0 : 1];
		++m_currentBlurUpsampleBlend;
	}

	float blurTolerance = 1.0f - powf(10.0f, m_blurTolerance) * 1920.0f / (float)loWidth;
	blurTolerance *= blurTolerance;
	float upsampleTolerance = powf(10.0f, m_upsampleTolerance);
	float noiseFilterWeight = 1.0f / (powf(10.0f, m_noiseFilterTolerance) + upsampleTolerance);

	kernel->GetParameter("InvLowResolution")->SetValueImmediate(XMFLOAT2(1.0f / loWidth, 1.0f / loHeight));
	kernel->GetParameter("InvHighResolution")->SetValueImmediate(XMFLOAT2(1.0f / hiWidth, 1.0f / hiHeight));
	kernel->GetParameter("NoiseFilterStrength")->SetValueImmediate(noiseFilterWeight);
	kernel->GetParameter("StepSize")->SetValueImmediate(1920.0f / (float)loWidth);
	kernel->GetParameter("kBlurTolerance")->SetValueImmediate(blurTolerance);
	kernel->GetParameter("kUpsampleTolerance")->SetValueImmediate(upsampleTolerance);

	commandList->TransitionResource(*destination, ResourceState::UnorderedAccess);
	commandList->TransitionResource(*loResDepth, ResourceState::NonPixelShaderResource);
	commandList->TransitionResource(*hiResDepth, ResourceState::NonPixelShaderResource);

	kernel->GetResource("AoResult")->SetUAVImmediate(destination);
	kernel->GetResource("LoResDB")->SetSRVImmediate(loResDepth);
	kernel->GetResource("HiResDB")->SetSRVImmediate(hiResDepth);

	if (interleavedAO != nullptr)
	{
		commandList->TransitionResource(*interleavedAO, ResourceState::NonPixelShaderResource);
		kernel->GetResource("LoResAO1")->SetSRVImmediate(interleavedAO);
	}

	if (highQualityAO != nullptr)
	{
		commandList->TransitionResource(*highQualityAO, ResourceState::NonPixelShaderResource);
		kernel->GetResource("LoResAO2")->SetSRVImmediate(highQualityAO);
	}

	if (hiResAO != nullptr)
	{
		commandList->TransitionResource(*hiResAO, ResourceState::NonPixelShaderResource);
		kernel->GetResource("HiResAO")->SetSRVImmediate(hiResAO);
	}

#if DX11
	commandList->SetShaderSampler(0, m_linearClampSampler.Get());
#endif
	
	kernel->Dispatch2D(commandList, hiWidth + 2, hiHeight + 2, 16, 16);
	kernel->UnbindSRVs(commandList);
	kernel->UnbindUAVs(commandList);
}