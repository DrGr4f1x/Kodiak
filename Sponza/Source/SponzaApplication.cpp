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
#include "Engine\Source\ColorBuffer.h"
#include "Engine\Source\CommandList.h"
#include "Engine\Source\CommonStates.h"
#include "Engine\Source\Defaults.h"
#include "Engine\Source\DepthBuffer.h"
#include "Engine\Source\Effect.h"
#include "Engine\Source\Format.h"
#include "Engine\Source\Log.h"
#include "Engine\Source\Material.h"
#include "Engine\Source\Model.h"
#include "Engine\Source\Renderer.h"
#include "Engine\Source\RenderPass.h"
#include "Engine\Source\RenderPipeline.h"
#include "Engine\Source\Scene.h"
#include "Engine\Source\StepTimer.h"


using namespace Kodiak;
using namespace DirectX;
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

#if 0
	auto idx = m_boxModel.AddMaterial(m_baseMaterial);
	m_boxModel.GetMesh(0)->SetMaterialIndex(idx);
#endif

	SetupScene();
	SetupPipeline();
}


void SponzaApplication::OnUpdate(StepTimer* timer)
{}


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
	baseEffect->SetDepthStencilState(CommonStates::DepthDefault());
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

	// TEMP: materials
	m_baseMaterial = make_shared<Material>();
	m_baseMaterial->SetName("Base");
	m_baseMaterial->SetEffect(baseEffect);

	m_depthMaterial = make_shared<Material>();
	m_depthMaterial->SetName("Depth");
	m_depthMaterial->SetEffect(depthEffect);

#if 0
	// Base effect
	auto effect = make_shared<Effect>();
	effect->SetName("Base effect");
	effect->SetVertexShaderPath("Engine", "SimpleVertexShader");
	effect->SetPixelShaderPath("Engine", "SimplePixelShader");
	effect->SetBlendState(CommonStates::Opaque());
	effect->SetDepthStencilState(CommonStates::DepthDefault());
	effect->SetRasterizerState(CommonStates::CullCounterClockwise());
	effect->Finalize();

	// Base material
	m_baseMaterial = make_shared<Material>();
	m_baseMaterial->SetName("Base material");
	m_baseMaterial->SetEffect(effect);
	m_baseMaterial->SetRenderPass(m_basePass);
#endif
}


void SponzaApplication::CreateModel()
{
	m_sponzaModel = LoadModel("sponza.h3d");
}


void SponzaApplication::SetupScene()
{
	// Setup scene camera
	m_camera = make_shared<Camera>();
	m_camera->SetPosition(XMFLOAT3(0.0f, 0.7f, 1.5f));
	m_camera->LookAt(XMFLOAT3(0.0f, -0.1f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_camera->SetPerspective(70.0f, static_cast<float>(m_width) / static_cast<float>(m_height), 0.01f, 100.0f);

	m_mainScene = make_shared<Scene>();

	// Add camera and model to scene
	m_mainScene->SetCamera(m_camera);
}


void SponzaApplication::SetupPipeline()
{
	auto pipeline = Renderer::GetInstance().GetRootPipeline();

	pipeline->SetRenderTarget(m_colorTarget, m_depthBuffer);
	pipeline->SetViewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f);
	pipeline->SetScissor(0, 0, m_width, m_height);
	pipeline->ClearColor(m_colorTarget);
	pipeline->ClearDepth(m_depthBuffer);

	pipeline->UpdateScene(m_mainScene);
	pipeline->RenderScene(m_mainScene);

	pipeline->Present(m_colorTarget);
}