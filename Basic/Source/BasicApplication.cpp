// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "BasicApplication.h"

#include "Engine\Source\Camera.h"
#include "Engine\Source\ColorBuffer.h"
#include "Engine\Source\CommandList.h"
#include "Engine\Source\CommonStates.h"
#include "Engine\Source\DepthBuffer.h"
#if 0
#include "Engine\Source\Effect.h"
#endif
#include "Engine\Source\Format.h"
#include "Engine\Source\Log.h"
#if 0
#include "Engine\Source\Material.h"
#endif
#include "Engine\Source\Model.h"
#include "Engine\Source\Renderer.h"
#include "Engine\Source\RenderPass.h"
#include "Engine\Source\RenderPipeline.h"
#include "Engine\Source\Scene.h"
#include "Engine\Source\StepTimer.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


BasicApplication::BasicApplication(uint32_t width, uint32_t height, const std::wstring& name)
	: Application(width, height, name)
{}


void BasicApplication::OnInit()
{
	LOG_INFO << "BasicApplication initialize";
	Renderer::GetInstance().SetWindow(m_width, m_height, m_hwnd);
	
	CreateResources();

	CreateMaterials();
	CreateModel();

#if 0
	auto idx = m_boxModel.AddMaterial(m_baseMaterial);
	m_boxModel.GetMesh(0)->SetMaterialIndex(idx);
#endif

	SetupScene();
	SetupPipeline();
}


void BasicApplication::OnUpdate(StepTimer* timer)
{}


void BasicApplication::OnDestroy()
{
	Renderer::GetInstance().Finalize();
	LOG_INFO << "BasicApplication finalize";
}


void BasicApplication::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_isTracking = true;
	m_mouseX = x;
	m_mouseY = y;
}


void BasicApplication::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (m_isTracking)
	{
		int deltaMouseX = x - m_mouseX;
		m_deltaRadians = XM_2PI * 2.0f * static_cast<float>(deltaMouseX) / static_cast<float>(m_width);
		
		XMFLOAT4X4 matrix;
		XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixRotationY(m_radians + m_deltaRadians)));

		m_boxModel->SetMatrix(matrix);
	}
}


void BasicApplication::OnMouseUp(WPARAM btnState, int x, int y)
{
	m_isTracking = false;
	m_radians += m_deltaRadians;
}


void BasicApplication::CreateResources()
{
	m_colorTarget = CreateColorBuffer("Main color buffer", m_width, m_height, 1, ColorFormat::R11G11B10_Float,
		DirectX::Colors::CornflowerBlue);

	m_depthBuffer = CreateDepthBuffer("Main depth buffer", m_width, m_height, DepthFormat::D32);
}


void BasicApplication::CreateMaterials()
{
	// Base render pass
	m_basePass = make_shared<RenderPass>("Base");
	m_basePass->SetRenderTargetFormat(ColorFormat::R11G11B10_Float, DepthFormat::D32);

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


void BasicApplication::CreateModel()
{
	m_boxModel = make_shared<StaticModel>();
	BoxMeshDesc meshDesc;
	meshDesc.colors[0] = XMFLOAT3(0.0f, 0.0f, 0.0f);
	meshDesc.colors[1] = XMFLOAT3(0.0f, 0.0f, 1.0f);
	meshDesc.colors[2] = XMFLOAT3(0.0f, 1.0f, 0.0f);
	meshDesc.colors[3] = XMFLOAT3(0.0f, 1.0f, 1.0f);
	meshDesc.colors[4] = XMFLOAT3(1.0f, 0.0f, 0.0f);
	meshDesc.colors[5] = XMFLOAT3(1.0f, 0.0f, 1.0f);
	meshDesc.colors[6] = XMFLOAT3(1.0f, 1.0f, 0.0f);
	meshDesc.colors[7] = XMFLOAT3(1.0f, 1.0f, 1.0f);
	meshDesc.genColors = true;

	auto mesh = MakeBoxMesh(meshDesc);
	
	m_boxModel->AddMesh(mesh);

	{
		auto mesh2 = mesh->Clone();

		XMFLOAT4X4 matrix;
		XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixTranslation(3.0f, 0.0f, 0.0f)));
		mesh2->SetMatrix(matrix);
		m_boxModel->AddMesh(mesh2);
	}

	{
		auto mesh2 = mesh->Clone();

		XMFLOAT4X4 matrix;
		XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixTranslation(-3.0f, 0.0f, 0.0f)));
		mesh2->SetMatrix(matrix);
		m_boxModel->AddMesh(mesh2);
	}

	{
		auto mesh2 = mesh->Clone();

		XMFLOAT4X4 matrix;
		XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixTranslation(0.0f, 0.0f, 3.0f)));
		mesh2->SetMatrix(matrix);
		m_boxModel->AddMesh(mesh2);
	}
	
	{
		auto mesh2 = mesh->Clone();

		XMFLOAT4X4 matrix;
		XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixTranslation(0.0f, 0.0f, -3.0f)));
		mesh2->SetMatrix(matrix);
		m_boxModel->AddMesh(mesh2);
	}
}


void BasicApplication::SetupScene()
{
	// Setup scene camera
	m_camera = make_shared<Camera>();
	m_camera->SetPosition(XMFLOAT3(0.0f, 0.7f, 1.5f));
	m_camera->LookAt(XMFLOAT3(0.0f, -0.1f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_camera->SetPerspective(70.0f, static_cast<float>(m_width) / static_cast<float>(m_height), 0.01f, 100.0f);

	m_mainScene = make_shared<Scene>();

	// Add camera and model to scene
	// TODO: decide on Renderer or RenderThread namespace
	//RenderThread::SetSceneCamera(m_mainScene, m_camera);
	m_mainScene->SetCamera(m_camera);
	m_mainScene->AddStaticModel(m_boxModel);
}


void BasicApplication::SetupPipeline()
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