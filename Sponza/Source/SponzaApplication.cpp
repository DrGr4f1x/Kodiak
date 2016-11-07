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
#include "Engine\Source\DeviceManager.h"
#include "Engine\Source\Effect.h"
#include "Engine\Source\Format.h"
#include "Engine\Source\FXAA.h"
#include "Engine\Source\InputState.h"
#include "Engine\Source\Log.h"
#include "Engine\Source\Material.h"
#include "Engine\Source\MaterialResource.h"
#include "Engine\Source\Model.h"
#include "Engine\Source\ParticleEffectManager.h"
#include "Engine\Source\ParticleEffect.h"
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
__itt_string_handle* itt_particles = nullptr;
__itt_string_handle* itt_postprocessing = nullptr;
__itt_string_handle* itt_shadows = nullptr;
__itt_string_handle* itt_debug_histogram = nullptr;
#endif


using namespace Kodiak;
using namespace Math;
using namespace DirectX;
using namespace std;


SponzaApplication::SponzaApplication(uint32_t width, uint32_t height, const std::wstring& name)
	: Application(width, height, name)
{}


void SponzaApplication::OnInit()
{
	LOG_INFO << "SponzaApplication initialize";
	DeviceManager::GetInstance().SetWindow(m_width, m_height, m_hwnd);

#if defined(PROFILING) && (PROFILING == 1)
	itt_frame_setup = __itt_string_handle_create("Frame setup");
	itt_depth_prepass = __itt_string_handle_create("Depth prepass");
	itt_ssao = __itt_string_handle_create("SSAO");
	itt_opaque_pass = __itt_string_handle_create("Opaque pass");
	itt_particles = __itt_string_handle_create("Particles");
	itt_postprocessing = __itt_string_handle_create("Postprocessing");
	itt_shadows = __itt_string_handle_create("Shadows");
	itt_debug_histogram = __itt_string_handle_create("Debug histogram");
#endif

	CreateResources();
	CreateParticleEffects();

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
	const float shadowDimX = 5000.0f;
	const float shadowDimY = 3000.0f;
	const float shadowDimZ = 3000.0f;
	m_shadowCamera->UpdateMatrix(-sunDirection, Vector3(0.0f, -500.0f, 0.0f), Vector3(shadowDimX, shadowDimY, shadowDimZ),
		m_shadowBuffer->GetWidth(), m_shadowBuffer->GetHeight(), 16);

	m_elapsedTime = static_cast<float>(timer->GetElapsedSeconds());
}


void SponzaApplication::OnRender()
{
	auto rootTask = SetupFrame();

	PresentParameters params;
	params.ToeStrength = 0.01f;
	params.PaperWhite = 200.0f;
	params.MaxBrightness = 600.0f;
	params.DebugMode = 0;

	const bool bHDRPresent = false;
	Renderer::GetInstance().Render(rootTask, bHDRPresent, params);
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
		DirectX::Colors::Black);

	m_depthBuffer = CreateDepthBuffer("Main depth buffer", m_width, m_height, DepthFormat::D32, 0.0f);

	m_linearDepthBuffer = CreateColorBuffer("Linear depth buffer", m_width, m_height, 1, ColorFormat::R16_Float, DirectX::Colors::Black);
	m_ssaoFullscreen = CreateColorBuffer("SSAO full res", m_width, m_height, 1, ColorFormat::R8_UNorm, DirectX::Colors::Black);

	m_lumaBuffer = make_shared<ColorBuffer>();
	m_lumaBuffer->Create("Luminance", m_width, m_height, 1, ColorFormat::R8_UNorm);

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
	m_postProcessing->LumaBuffer = m_lumaBuffer;
	m_postProcessing->EnableAdaptation = true;
	m_postProcessing->EnableBloom = true;
	
	m_fxaa = make_shared<FXAA>();
	m_fxaa->Initialize(m_width, m_height);
	m_fxaa->SceneColorBuffer = m_colorTarget;
	m_fxaa->LumaBuffer = m_lumaBuffer;
	m_fxaa->DebugDraw = false;
	m_fxaa->UsePrecomputedLuma = true;

	if (!DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{
		m_postEffectsBuffer = CreateColorBuffer("Post Effects Buffer", m_width, m_height, 1, ColorFormat::R32_UInt, DirectX::Colors::Black);
		m_postProcessing->PostEffectsBuffer = m_postEffectsBuffer;
		m_fxaa->PostEffectsBuffer = m_postEffectsBuffer;
	}

	m_shadowBuffer = make_shared<ShadowBuffer>();
	m_shadowBuffer->Create("Shadow Map", 2048, 2048);
}


void SponzaApplication::CreateParticleEffects()
{
	m_particleEffectManager = make_shared<ParticleEffectManager>();
	m_particleEffectManager->Initialize(m_width, m_height);

	ParticleEffectProperties effect = ParticleEffectProperties();
	effect.MinStartColor = effect.MaxStartColor = effect.MinEndColor = effect.MaxEndColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
	effect.TexturePath = "sparkTex.dds";

	effect.TotalActiveLifetime = FLT_MAX;
	effect.Size = Vector4(4.0f, 8.0f, 4.0f, 8.0f);
	effect.Velocity = Vector4(20.0f, 200.0f, 50.0f, 180.0f);
	effect.LifeMinMax = XMFLOAT2(1.0f, 3.0f);
	effect.MassMinMax = XMFLOAT2(4.5f, 15.0f);
	effect.EmitProperties.Gravity = XMFLOAT3(0.0f, -100.0f, 0.0f);
	effect.EmitProperties.FloorHeight = -0.5f;
	effect.EmitProperties.EmitPosW = effect.EmitProperties.LastEmitPosW = XMFLOAT3(-1200.0f, 185.0f, -445.0f);
	effect.EmitProperties.MaxParticles = 800;
	effect.EmitRate = 64.0f;
	effect.Spread.x = 20.0f;
	effect.Spread.y = 50.0f;
	m_particleEffectManager->InstantiateEffect(&effect);

	ParticleEffectProperties smoke = ParticleEffectProperties();
	smoke.TexturePath = "smoke.dds";

	smoke.TotalActiveLifetime = FLT_MAX;;
	smoke.EmitProperties.MaxParticles = 25;
	smoke.EmitProperties.EmitPosW = smoke.EmitProperties.LastEmitPosW = XMFLOAT3(1120.0f, 185.0f, -445.0f);
	smoke.EmitRate = 64.0f;
	smoke.LifeMinMax = XMFLOAT2(2.5f, 4.0f);
	smoke.Size = Vector4(60.0f, 108.0f, 30.0f, 208.0f);
	smoke.Velocity = Vector4(30.0f, 30.0f, 10.0f, 40.0f);
	smoke.MassMinMax = XMFLOAT2(1.0, 3.5);
	smoke.Spread.x = 60.0f;
	smoke.Spread.y = 70.0f;
	smoke.Spread.z = 20.0f;
	m_particleEffectManager->InstantiateEffect(&smoke);

	ParticleEffectProperties fire = ParticleEffectProperties();
	fire.MinStartColor = fire.MaxStartColor = fire.MinEndColor = fire.MaxEndColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
	fire.TexturePath = "fire.dds";

	fire.TotalActiveLifetime = FLT_MAX;
	fire.Size = Vector4(54.0f, 68.0f, 0.1f, 0.3f);
	fire.Velocity = Vector4(10.0f, 30.0f, 50.0f, 50.0f);
	fire.LifeMinMax = XMFLOAT2(1.0f, 3.0f);
	fire.MassMinMax = XMFLOAT2(10.5f, 14.0f);
	fire.EmitProperties.Gravity = XMFLOAT3(0.0f, 1.0f, 0.0f);
	fire.EmitProperties.EmitPosW = fire.EmitProperties.LastEmitPosW = XMFLOAT3(1120.0f, 125.0f, 405.0f);
	fire.EmitProperties.MaxParticles = 25;
	fire.EmitRate = 64.0f;
	fire.Spread.x = 1.0f;
	fire.Spread.y = 60.0f;
	m_particleEffectManager->InstantiateEffect(&fire);
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
	baseEffect->SetVertexShaderPath("BaseVS");
	baseEffect->SetPixelShaderPath("BasePS");
	baseEffect->SetBlendState(CommonStates::Opaque());
	baseEffect->SetRasterizerState(CommonStates::CullCounterClockwise());
	baseEffect->SetDepthStencilState(CommonStates::DepthReadEqual());
	baseEffect->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	baseEffect->SetRenderTargetFormat(ColorFormat::R11G11B10_Float, DepthFormat::D32);
	baseEffect->Finalize();
	SetDefaultBaseEffect(baseEffect);

	auto depthEffect = make_shared<Effect>("Depth");
	depthEffect->SetVertexShaderPath("DepthVS");
	depthEffect->SetPixelShaderPath("DepthPS");
	depthEffect->SetBlendState(CommonStates::Opaque());
	depthEffect->SetRasterizerState(CommonStates::CullCounterClockwise());
	depthEffect->SetDepthStencilState(CommonStates::DepthGreaterEqual());
	depthEffect->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	depthEffect->SetRenderTargetFormats(0, nullptr, DepthFormat::D32);
	depthEffect->Finalize();
	SetDefaultDepthEffect(depthEffect);

	auto shadowEffect = make_shared<Effect>("Shadow");
	shadowEffect->SetVertexShaderPath("DepthVS");
	shadowEffect->SetPixelShaderPath("DepthPS");
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
			material->GetResource("texSSAO")->SetSRV(*m_ssaoFullscreen);
			material->GetResource("texShadow")->SetSRV(*m_shadowBuffer);
		}
	}
#endif
}


void SponzaApplication::SetupScene()
{
	// Setup scene camera
	m_camera = make_shared<Camera>();
	m_camera->SetEyeAtUp(Vector3(1099.0f, 652.0f, -39.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(kYUnitVector));
	m_camera->SetZRange(1.0f, 10000.0f);

	m_cameraController = make_shared<CameraController>(m_camera, m_inputState, Vector3(kYUnitVector));

	m_ssao->SetCamera(m_camera);

	m_shadowCamera = make_shared<ShadowCamera>();
	const Vector3 sunDirection = Vector3(0.336f, 0.924f, -0.183f);
	const float shadowDimX = 5000.0f;
	const float shadowDimY = 3000.0f;
	const float shadowDimZ = 3000.0f;
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

		m_mainScene->RenderShadows(GetDefaultShadowPass(), commandList);

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

			if(true)
			{
				PROFILE_BEGIN(itt_particles);

				auto compCommandList = commandList->GetComputeCommandList();
				m_particleEffectManager->Update(*compCommandList, m_elapsedTime);
				m_particleEffectManager->Render(*commandList, m_camera, m_colorTarget, m_depthBuffer, m_linearDepthBuffer);

				PROFILE_END();
			}

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

	auto fxaaTask = make_shared<RenderTask>();
	fxaaTask->SetName("FXAA");
	fxaaTask->Render = [this]
	{
		auto commandList = GraphicsCommandList::Begin();

		m_fxaa->Render(commandList);

		commandList->CloseAndExecute();
	};
	postTask->Continue(fxaaTask);

	if (!DeviceManager::GetInstance().SupportsTypedUAVLoad_R11G11B10_FLOAT())
	{
		auto finalizePostTask = make_shared<RenderTask>();
		finalizePostTask->SetName("Finalize Post");

		finalizePostTask->Render = [this]
		{
			auto commandList = GraphicsCommandList::Begin();

			m_postProcessing->FinalizePostProcessing(commandList);

			commandList->CloseAndExecute();
		};
		fxaaTask->Continue(finalizePostTask);
	}

	rootTask->Present(m_colorTarget);

	return rootTask;
}