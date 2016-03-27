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

#include "CameraController.h"
#include "Engine\Source\Camera.h"
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
#include "Engine\Source\MaterialParameter.h"
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
{
	for (uint32_t i = 0; i < 4; ++i)
	{
		m_meshRadians[i] = 0.0f;
	}
}


void BasicApplication::OnInit()
{
	LOG_INFO << "BasicApplication initialize";
	Renderer::GetInstance().SetWindow(m_width, m_height, m_hwnd);
	
	CreateResources();

	CreateMaterials();
	CreateModel();

	SetupScene();
	SetupPipeline();
}


void BasicApplication::OnUpdate(StepTimer* timer)
{
	if (m_inputState->IsFirstPressed(InputState::kKey_escape))
	{
		PostQuitMessage(0);
	}

	m_cameraController->Update(static_cast<float>(timer->GetElapsedSeconds()));

	auto seconds = static_cast<float>(timer->GetTotalSeconds());
	seconds *= 0.5f;

	{
		auto mesh = m_boxModel->GetMesh(1);
		float deltaRadians = max(0.0f, 5.0f * (sinf(seconds) - 0.8f));
		
		if (deltaRadians > 0.0f)
		{
			m_meshRadians[0] += deltaRadians;

			XMFLOAT4X4 matrix;
			XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixRotationZ(deltaRadians)));

			mesh->ConcatenateMatrix(matrix);
		}

		auto v = 0.5f * sinf(seconds) + 0.5f;
	}

	{
		auto mesh = m_boxModel->GetMesh(2);
		float deltaRadians = max(0.0f, 5.0f * (sinf(max(0.0f, seconds - 1.0f)) - 0.8f));

		if (deltaRadians > 0.0f)
		{
			m_meshRadians[1] += deltaRadians;

			XMFLOAT4X4 matrix;
			XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixRotationZ(-deltaRadians)));

			mesh->ConcatenateMatrix(matrix);
		}

		auto v = 0.5f * sinf(seconds - 1.0f) + 0.5f;
	}

	{
		auto mesh = m_boxModel->GetMesh(3);
		float deltaRadians = max(0.0f, 5.0f * (sinf(max(0.0f, seconds - 2.0f)) - 0.8f));

		if (deltaRadians > 0.0f)
		{
			m_meshRadians[2] += deltaRadians;

			XMFLOAT4X4 matrix;
			XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixRotationX(deltaRadians)));

			mesh->ConcatenateMatrix(matrix);
		}

		auto v = 0.5f * sinf(seconds - 2.0f) + 0.5f;
	}

	{
		auto mesh = m_boxModel->GetMesh(4);
		float deltaRadians = max(0.0f, 5.0f * (sinf(max(0.0f, seconds - 3.0f)) - 0.8f));

		if (deltaRadians > 0.0f)
		{
			m_meshRadians[3] += deltaRadians;

			XMFLOAT4X4 matrix;
			XMStoreFloat4x4(&matrix, XMMatrixTranspose(XMMatrixRotationX(-deltaRadians)));

			mesh->ConcatenateMatrix(matrix);
		}

		auto v = 0.5f * sinf(seconds - 3.0f) + 0.5f;
	}
}


void BasicApplication::OnDestroy()
{
	Renderer::GetInstance().Finalize();
	LOG_INFO << "BasicApplication finalize";
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
	auto basePass = make_shared<RenderPass>("Base");
	basePass->SetRenderTargetFormat(ColorFormat::R11G11B10_Float, DepthFormat::D32);
	SetDefaultBasePass(basePass);

	// Base effect
	auto effect = make_shared<Effect>();
	effect->SetName("Base effect");
	effect->SetVertexShaderPath("Engine", "SimpleVertexShader.cso");
	effect->SetPixelShaderPath("Engine", "SimplePixelShader.cso");
	effect->SetBlendState(CommonStates::Opaque());
	effect->SetDepthStencilState(CommonStates::DepthDefault());
	effect->SetRasterizerState(CommonStates::CullClockwise());
	effect->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	effect->SetRenderTargetFormat(ColorFormat::R11G11B10_Float, DepthFormat::D32);
	effect->Finalize();
	SetDefaultBaseEffect(effect);
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

	m_cameraController = make_shared<CameraController>(m_camera, m_inputState, XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_mainScene = make_shared<Scene>();

	// Add camera and model to scene
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
	pipeline->RenderScenePass(GetDefaultBasePass(), m_mainScene);

	pipeline->Present(m_colorTarget);
}