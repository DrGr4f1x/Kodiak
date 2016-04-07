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

#include "ComputeKernel.h"
#include "Renderer.h"
#include "RenderTask.h"


using namespace Kodiak;
using namespace std;


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


shared_ptr<RenderTask> SSAO::GetRenderTask()
{
	auto renderTask = make_shared<RenderTask>();
	renderTask->SetName("SSAO");

	return renderTask;
}


void SSAO::SetEnabled(bool enabled)
{
	auto thisSSAO = shared_from_this();
	Renderer::GetInstance().EnqueueTask([thisSSAO, enabled](RenderTaskEnvironment& rte)
	{
		thisSSAO->m_enabled = enabled;
	});
}


void SSAO::SetDebugDraw(bool enabled)
{
	auto thisSSAO = shared_from_this();
	Renderer::GetInstance().EnqueueTask([thisSSAO, enabled](RenderTaskEnvironment& rte)
	{
		thisSSAO->m_debugDraw = enabled;
	});
}