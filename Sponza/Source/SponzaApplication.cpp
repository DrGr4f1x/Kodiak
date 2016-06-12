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
#include "Engine\Source\MaterialResource.h"
#include "Engine\Source\Model.h"
#include "Engine\Source\PostProcessing.h"
#include "Engine\Source\Profile.h"
#include "Engine\Source\Renderer.h"
#include "Engine\Source\RenderPass.h"
#include "Engine\Source\RenderTask.h"
#include "Engine\Source\SSAO.h"
#include "Engine\Source\Scene.h"
#include "Engine\Source\ShadowBuffer.h"
#include "Engine\Source\ShadowCamera.h"
#include "Engine\Source\StepTimer.h"


#if defined(PROFILING) && (PROFILING == 1)
#include "IntelITT\include\ittnotify.h"
__itt_string_handle* itt_frame_setup = nullptr;
__itt_string_handle* itt_depth_prepass = nullptr;
__itt_string_handle* itt_ssao = nullptr;
__itt_string_handle* itt_opaque_pass = nullptr;
__itt_string_handle* itt_postprocessing = nullptr;
__itt_string_handle* itt_shadows = nullptr;
#endif


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

#if defined(PROFILING) && (PROFILING == 1)
	itt_frame_setup = __itt_string_handle_create("Frame setup");
	itt_depth_prepass = __itt_string_handle_create("Depth prepass");
	itt_ssao = __itt_string_handle_create("SSAO");
	itt_opaque_pass = __itt_string_handle_create("Opaque pass");
	itt_postprocessing = __itt_string_handle_create("Postprocessing");
	itt_shadows = __itt_string_handle_create("Shadows");
#endif

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
	const Vector3 sunDirection = Vector3(0.336f, 0.924f, -0.183f);
	const uint32_t shadowDimX = 5000;
	const uint32_t shadowDimY = 3000;
	const uint32_t shadowDimZ = 3000;
	m_shadowCamera->UpdateMatrix(-sunDirection, Vector3(0.0f, -500.0f, 0.0f), Vector3(shadowDimX, shadowDimY, shadowDimZ),
		m_shadowBuffer->GetWidth(), m_shadowBuffer->GetHeight(), 16);
}


void SponzaApplication::OnRender()
{
	auto rootTask = SetupFrame();
	Renderer::GetInstance().Render(rootTask);
}


void SponzaApplication::OnDestroy()
{
	Renderer::GetInstance().Finalize();

	SetDefaultBasePass(nullptr);
	SetDefaultDepthPass(nullptr);
	
	SetDefaultBaseEffect(nullptr);
	SetDefaultDepthEffect(nullptr);

	LOG_INFO << "SponzaApplication finalize";
}


void SponzaApplication::CreateResources()
{
	m_colorTarget = CreateColorBuffer("Main color buffer", m_width, m_height, 1, ColorFormat::R11G11B10_Float,
		DirectX::Colors::CornflowerBlue);

	m_depthBuffer = CreateDepthBuffer("Main depth buffer", m_width, m_height, DepthFormat::D32, 0.0f);

	m_linearDepthBuffer = CreateColorBuffer("Linear depth buffer", m_width, m_height, 1, ColorFormat::R16_Float, DirectX::Colors::Black);
	m_ssaoFullscreen = CreateColorBuffer("SSAO full res", m_width, m_height, 1, ColorFormat::R8_UNorm, DirectX::Colors::Black);

	m_ssao = make_shared<SSAO>();
	m_ssao->Initialize(m_width, m_height);
	m_ssao->SceneColorBuffer = m_colorTarget;
	m_ssao->SceneDepthBuffer = m_depthBuffer;
	m_ssao->LinearDepthBuffer = m_linearDepthBuffer;
	m_ssao->SsaoFullscreen = m_ssaoFullscreen;

	m_ssao->Enable = true;
	m_ssao->DebugDraw = false;

	m_postProcessing = make_shared<PostProcessing>();
	m_postProcessing->Initialize(m_width, m_height);
	m_postProcessing->SceneColorBuffer = m_colorTarget;
	m_postProcessing->EnableAdaptation = true;
	//m_postProcessing->EnableBloom = false;

	m_shadowBuffer = make_shared<ShadowBuffer>();
	m_shadowBuffer->Create("Shadow Map", 2048, 2048);
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

	auto shadowPass = make_shared<RenderPass>("Shadow");
	shadowPass->SetDepthTargetFormat(DepthFormat::D16);
	SetDefaultShadowPass(shadowPass);


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
	depthEffect->SetDepthStencilState(CommonStates::DepthGreaterEqual());
	depthEffect->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	depthEffect->SetRenderTargetFormats(0, nullptr, DepthFormat::D32);
	depthEffect->Finalize();
	SetDefaultDepthEffect(depthEffect);

	auto shadowEffect = make_shared<Effect>("Shadow");
	shadowEffect->SetVertexShaderPath(ShaderPath("DepthVS.cso"));
	shadowEffect->SetPixelShaderPath(ShaderPath("DepthPS.cso"));
	shadowEffect->SetBlendState(CommonStates::Opaque());
	shadowEffect->SetRasterizerState(CommonStates::Shadow());
	shadowEffect->SetDepthStencilState(CommonStates::DepthGreaterEqual());
	shadowEffect->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	shadowEffect->SetRenderTargetFormats(0, nullptr, DepthFormat::D16);
	shadowEffect->Finalize();
	SetDefaultShadowEffect(shadowEffect);
}


void SponzaApplication::CreateModel()
{
	m_sponzaModel = LoadModel("sponza.h3d");

	// Miserable hack!!!!
#if DX12
	const auto numMeshes = m_sponzaModel->GetNumMeshes();
	for (uint32_t i = 0; i < numMeshes; ++i)
	{
		auto mesh = m_sponzaModel->GetMesh(i);

		const auto numParts = mesh->GetNumMeshParts();
		for (uint32_t j = 0; j < numParts; ++j)
		{
			auto material = mesh->GetMaterial(j);
			material->GetResource("texSSAO")->SetSRV(m_ssaoFullscreen);
			material->GetResource("texShadow")->SetSRV(m_shadowBuffer);
		}
	}
#endif
}


void SponzaApplication::SetupScene()
{
	// Setup scene camera
	m_camera = make_shared<Camera>();
	m_camera->SetEyeAtUp(Vector3(1099.0f, 652.0f, -39.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(kYUnitVector));
	m_camera->SetPerspectiveMatrix(45.0f, static_cast<float>(m_height) / static_cast<float>(m_width), 1.0f, 10000.0f);
	
	m_cameraController = make_shared<CameraController>(m_camera, m_inputState, Vector3(kYUnitVector));

	m_ssao->SetCamera(m_camera);

	m_shadowCamera = make_shared<ShadowCamera>();
	const Vector3 sunDirection = Vector3(0.336f, 0.924f, -0.183f);
	const uint32_t shadowDimX = 5000;
	const uint32_t shadowDimY = 3000;
	const uint32_t shadowDimZ = 3000;
	m_shadowCamera->UpdateMatrix(-sunDirection, Vector3(0.0f, -500.0f, 0.0f), Vector3(shadowDimX, shadowDimY, shadowDimZ),
		m_shadowBuffer->GetWidth(), m_shadowBuffer->GetHeight(), 16);

	m_mainScene = make_shared<Scene>();

	// Add camera and model to scene
	m_mainScene->SetCamera(m_camera);
	m_mainScene->SetShadowBuffer(m_shadowBuffer);
	m_mainScene->SetShadowCamera(m_shadowCamera);
#if DX11
	m_mainScene->SsaoFullscreen = m_ssaoFullscreen;
#endif
	m_mainScene->AddStaticModel(m_sponzaModel);
}


shared_ptr<RootRenderTask> SponzaApplication::SetupFrame()
{
	auto rootTask = make_shared<RootRenderTask>();
	rootTask->SetName("Root task");
	rootTask->Render = [this]
	{
		PROFILE_BEGIN(itt_frame_setup);
		auto commandList = GraphicsCommandList::Begin();

		commandList->SetRenderTarget(*m_colorTarget, *m_depthBuffer);
		commandList->SetViewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f);
		commandList->SetScissor(0, 0, m_width, m_height);
		commandList->ClearColor(*m_colorTarget);
		commandList->ClearDepth(*m_depthBuffer);

		commandList->CloseAndExecute();
		PROFILE_END();
	};
	

	auto depthTask = make_shared<RenderTask>();
	depthTask->SetName("Depth prepass");
	depthTask->Render = [this]
	{
		PROFILE_BEGIN(itt_depth_prepass);

		auto commandList = GraphicsCommandList::Begin();

		commandList->SetViewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f);
		commandList->SetScissor(0, 0, m_width, m_height);
		commandList->SetDepthStencilTarget(*m_depthBuffer);

		m_mainScene->Update(commandList);
		m_mainScene->Render(GetDefaultDepthPass(), commandList);
		
		commandList->UnbindRenderTargets();

		commandList->CloseAndExecute();

		PROFILE_END();
	};
	rootTask->Continue(depthTask);


	bool ssaoEnabled = true;

	auto ssaoTask = make_shared<RenderTask>();
	ssaoTask->SetName("SSAO");
	ssaoTask->SetEnabled(ssaoEnabled);
	ssaoTask->Render = [this]
	{
		PROFILE_BEGIN(itt_ssao);

		auto commandList = GraphicsCommandList::Begin();

		m_ssao->Render(commandList);

		commandList->CloseAndExecute();

		PROFILE_END();
	};
	depthTask->Continue(ssaoTask);


	auto shadowTask = make_shared<RenderTask>();
	shadowTask->SetName("Shadows");
	shadowTask->Render = [this]
	{
		PROFILE_BEGIN(itt_shadows);

		auto commandList = GraphicsCommandList::Begin();
		commandList->PIXBeginEvent("Shadows");

		m_shadowBuffer->BeginRendering(*commandList);

		m_mainScene->RenderShadows(GetDefaultShadowPass(), m_shadowCamera->GetViewProjMatrix(), commandList);

		m_shadowBuffer->EndRendering(*commandList);

		commandList->PIXEndEvent();
		commandList->CloseAndExecute();

		PROFILE_END();
	};
	ssaoTask->Continue(shadowTask);


	auto opaqueTask = make_shared<RenderTask>();
	opaqueTask->SetName("Opaque pass");
	opaqueTask->Render = [this, ssaoEnabled]
	{
		if (!m_ssao->DebugDraw || !ssaoEnabled)
		{
			PROFILE_BEGIN(itt_opaque_pass);
			auto commandList = GraphicsCommandList::Begin();

			commandList->SetViewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f);
			commandList->SetScissor(0, 0, m_width, m_height);
			commandList->SetRenderTarget(*m_colorTarget, *m_depthBuffer);

			m_mainScene->Update(commandList);
			m_mainScene->Render(GetDefaultBasePass(), commandList);

			commandList->CloseAndExecute();
			PROFILE_END();
		}
	};
	shadowTask->Continue(opaqueTask);


	auto postTask = make_shared<RenderTask>();
	postTask->SetName("Postprocessing");
	postTask->Render = [this]
	{
		PROFILE_BEGIN(itt_postprocessing);

		auto commandList = GraphicsCommandList::Begin();

		m_postProcessing->Render(commandList);

		commandList->CloseAndExecute();

		PROFILE_END();
	};
	opaqueTask->Continue(postTask);


	rootTask->Present(m_colorTarget);

	return rootTask;
}