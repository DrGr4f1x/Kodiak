// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "SponzaApplication.h"

#include "Engine\Source\Camera.h"
#include "Engine\Source\CameraController.h"
#include "Engine\Source\ColorBuffer.h"
#include "Engine\Source\CommandList.h"
#include "Engine\Source\CommonStates.h"
#include "Engine\Source\Defaults.h"
#include "Engine\Source\DepthBuffer.h"
#include "Engine\Source\Effect.h"
#include "Engine\Source\Format.h"
#include "Engine\Source\InputState.h"
#include "Engine\Source\Log.h"
#include "Engine\Source\Material.h"
#include "Engine\Source\Model.h"
#include "Engine\Source\Renderer.h"
#include "Engine\Source\RenderPass.h"
#include "Engine\Source\RenderTask.h"
#include "Engine\Source\SSAO.h"
#include "Engine\Source\Scene.h"
#include "Engine\Source\StepTimer.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


SponzaApplication::SponzaApplication(uint32_t width, uint32_t height, const std::wstring& name)
	: Application(width, height, name)
{}


void SponzaApplication::OnInit()
{
	LOG_INFO << "SponzaApplication initialize";
	Renderer::GetInstance().SetWindow(m_width, m_height, m_hwnd);

	CreateResources();

	CreateEffects();
	CreateModel();

	SetupScene();
}


void SponzaApplication::OnUpdate(StepTimer* timer)
{
	if (m_inputState->IsFirstPressed(InputState::kKey_escape))
	{
		PostQuitMessage(0);
	}

	m_cameraController->Update(static_cast<float>(timer->GetElapsedSeconds()));
}


void SponzaApplication::OnRender()
{
	auto rootTask = SetupFrame();
	Renderer::GetInstance().Render(rootTask);
}


void SponzaApplication::OnDestroy()
{
	SetDefaultBasePass(nullptr);
	SetDefaultDepthPass(nullptr);
	
	SetDefaultBaseEffect(nullptr);
	SetDefaultDepthEffect(nullptr);

	Renderer::GetInstance().Finalize();
	LOG_INFO << "SponzaApplication finalize";
}


void SponzaApplication::CreateResources()
{
	m_colorTarget = CreateColorBuffer("Main color buffer", m_width, m_height, 1, ColorFormat::R11G11B10_Float,
		DirectX::Colors::CornflowerBlue);

	m_depthBuffer = CreateDepthBuffer("Main depth buffer", m_width, m_height, DepthFormat::D32);

	m_ssao = make_shared<SSAO>();
	m_ssao->Initialize();
}


void SponzaApplication::CreateEffects()
{
	// Default render passes
	auto basePass = make_shared<RenderPass>("Base");
	basePass->SetRenderTargetFormat(ColorFormat::R11G11B10_Float, DepthFormat::D32);
	SetDefaultBasePass(basePass);

	auto depthPass = make_shared<RenderPass>("Depth");
	depthPass->SetDepthTargetFormat(DepthFormat::D32);
	SetDefaultDepthPass(depthPass);

	// Default effects
	auto baseEffect = make_shared<Effect>("Base");
	baseEffect->SetVertexShaderPath(ShaderPath("BaseVS.cso"));
	baseEffect->SetPixelShaderPath(ShaderPath("BasePS.cso"));
	baseEffect->SetBlendState(CommonStates::Opaque());
	baseEffect->SetRasterizerState(CommonStates::CullCounterClockwise());
	baseEffect->SetDepthStencilState(CommonStates::DepthReadEqual());
	baseEffect->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	baseEffect->SetRenderTargetFormat(ColorFormat::R11G11B10_Float, DepthFormat::D32);
	baseEffect->Finalize();
	SetDefaultBaseEffect(baseEffect);

	auto depthEffect = make_shared<Effect>("Depth");
	depthEffect->SetVertexShaderPath(ShaderPath("DepthVS.cso"));
	depthEffect->SetPixelShaderPath(ShaderPath("DepthPS.cso"));
	depthEffect->SetBlendState(CommonStates::Opaque());
	depthEffect->SetRasterizerState(CommonStates::CullCounterClockwise());
	depthEffect->SetDepthStencilState(CommonStates::DepthDefault());
	depthEffect->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	depthEffect->SetRenderTargetFormats(0, nullptr, DepthFormat::D32);
	depthEffect->Finalize();
	SetDefaultDepthEffect(depthEffect);
}


void SponzaApplication::CreateModel()
{
	m_sponzaModel = LoadModel("sponza.h3d");
}


void SponzaApplication::SetupScene()
{
	// Setup scene camera
	m_camera = make_shared<Camera>();
	m_camera->SetPosition(Vector3(1099.0f, 652.0f, -39.0f));
	m_camera->LookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(kYUnitVector));
	m_camera->SetPerspective(45.0f, static_cast<float>(m_width) / static_cast<float>(m_height), 1.0f, 10000.0f);

	m_cameraController = make_shared<CameraController>(m_camera, m_inputState, Vector3(kYUnitVector));

	m_mainScene = make_shared<Scene>();

	// Add camera and model to scene
	m_mainScene->SetCamera(m_camera);
	m_mainScene->AddStaticModel(m_sponzaModel);
}


shared_ptr<RootRenderTask> SponzaApplication::SetupFrame()
{
	auto rootTask = make_shared<RootRenderTask>();
	rootTask->SetName("Root task");

	rootTask->SetRenderTarget(m_colorTarget, m_depthBuffer);
	rootTask->SetViewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f);
	rootTask->SetScissor(0, 0, m_width, m_height);
	rootTask->ClearColor(m_colorTarget);
	rootTask->ClearDepth(m_depthBuffer);

	auto depthTask = make_shared<RenderTask>();
	depthTask->SetName("Depth prepass");
	depthTask->SetViewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f);
	depthTask->SetScissor(0, 0, m_width, m_height);
	depthTask->UpdateScene(m_mainScene);
	depthTask->SetDepthStencilTarget(m_depthBuffer);
	depthTask->RenderScenePass(GetDefaultDepthPass(), m_mainScene);
	rootTask->Continue(depthTask);

	auto opaqueTask = make_shared<RenderTask>();
	opaqueTask->SetName("Opaque pass");
	opaqueTask->SetViewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f);
	opaqueTask->SetScissor(0, 0, m_width, m_height);
	opaqueTask->UpdateScene(m_mainScene);
	opaqueTask->SetRenderTarget(m_colorTarget, m_depthBuffer);
	opaqueTask->RenderScenePass(GetDefaultBasePass(), m_mainScene);
	depthTask->Continue(opaqueTask);

	rootTask->Present(m_colorTarget);

	return rootTask;
}