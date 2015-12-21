// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Scene.h"

#include "Camera.h"
#include "CommandList.h"
#include "ConstantBuffer.h"
#include "Format.h"
#include "PipelineState.h"
#include "Model.h"
#include "RenderEnums.h"
#include "Shader.h"
#include "ShaderManager.h"

#if defined(DX12)
#include "RootSignature12.h"
#endif

using namespace Kodiak;
using namespace std;
using namespace DirectX;
using namespace RenderThread;


Scene::Scene()
{
	XMStoreFloat4x4(&m_modelTransform, XMMatrixIdentity());
	Initialize();
}


void Scene::AddModel(shared_ptr<Model> model)
{
	m_models.push_back(model);
}


void Scene::Update(GraphicsCommandList& commandList)
{
	// Update per-view constants
	XMMATRIX xmProjection = XMLoadFloat4x4(&m_camera->GetProjectionMatrix());
	XMStoreFloat4x4(&m_perViewConstants.projection, XMMatrixTranspose(xmProjection));

	XMMATRIX xmView = XMLoadFloat4x4(&m_camera->GetViewMatrix());
	XMStoreFloat4x4(&m_perViewConstants.view, XMMatrixTranspose(xmView));

	auto perViewData = commandList.MapConstants(*m_perViewConstantBuffer);
	memcpy(perViewData, &m_perViewConstants, sizeof(PerViewConstants));
	commandList.UnmapConstants(*m_perViewConstantBuffer);

	// Visit models (HACK)
	for (auto model : m_models)
	{
		if (model->IsDirty())
		{
			m_perObjectConstants.model = model->GetTransform();

			// Update per-object constants (HACK)
			auto perObjectData = commandList.MapConstants(*m_perObjectConstantBuffer);
			memcpy(perObjectData, &m_perObjectConstants, sizeof(PerObjectConstants));
			commandList.UnmapConstants(*m_perObjectConstantBuffer);

			model->ResetDirty();
		}
	}
}


void Scene::Render(GraphicsCommandList& commandList)
{
	// Visit models
	for (auto model : m_models)
	{
		// Visit meshes
		for (auto mesh : model->m_meshes)
		{
			// Visit mesh parts
			for (auto meshPart : mesh.m_meshParts)
			{
				commandList.SetPipelineState(*m_pso);
#if defined(DX12)
				commandList.SetRootSignature(*m_rootSignature);
				commandList.SetConstantBuffer(0, *m_perViewConstantBuffer);
				commandList.SetConstantBuffer(1, *m_perObjectConstantBuffer);
#elif defined(DX11)
				commandList.SetVertexShaderConstants(0, *m_perViewConstantBuffer);
				commandList.SetVertexShaderConstants(1, *m_perObjectConstantBuffer);
#endif

				commandList.SetVertexBuffer(0, *meshPart.m_vertexBuffer);
				commandList.SetIndexBuffer(*meshPart.m_indexBuffer);
				commandList.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				commandList.DrawIndexed(meshPart.m_indexCount, meshPart.m_startIndex, meshPart.m_baseVertexOffset);
			}
		}
	}
}


void Scene::SetCamera(shared_ptr<Kodiak::Camera> camera)
{
	if (!camera->GetProxy())
	{
		camera->CreateProxy();
	}

	m_camera = camera->GetProxy();
}


void Scene::Initialize()
{
	using namespace DirectX;

	XMStoreFloat4x4(&m_perObjectConstants.model, XMMatrixIdentity());

	// Default blend state - no blend
	BlendStateDesc defaultBlendState;

	// Depth-stencil state - normal depth testing
	DepthStencilStateDesc depthStencilState(true, true);

	// Rasterizer state - two-sided
	RasterizerStateDesc rasterizerState(CullMode::Back, FillMode::Solid);

	// Load shaders
	auto vs = ShaderManager::GetInstance().LoadVertexShader(ShaderPath("Engine", "SimpleVertexShader.cso"));
	auto ps = ShaderManager::GetInstance().LoadPixelShader(ShaderPath("Engine", "SimplePixelShader.cso"));
	(vs->loadTask && ps->loadTask).wait();

#if defined(DX12)
	// Configure root signature
	m_rootSignature = make_shared<RootSignature>(2);
	(*m_rootSignature)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	(*m_rootSignature)[1].InitAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_VERTEX);
	m_rootSignature->Finalize(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
#endif

	// Configure PSO
	m_pso = make_shared<GraphicsPSO>();
#if defined(DX12)
	m_pso->SetRootSignature(*m_rootSignature);
#endif
	m_pso->SetBlendState(defaultBlendState);
	m_pso->SetRasterizerState(rasterizerState);
	m_pso->SetDepthStencilState(depthStencilState);
	m_pso->SetInputLayout(*vs->GetInputLayout());
#if defined(DX12)
	m_pso->SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	m_pso->SetRenderTargetFormat(ColorFormat::R11G11B10_Float, DepthFormat::D32);
#endif
	m_pso->SetVertexShader(vs.get());
	m_pso->SetPixelShader(ps.get());

	m_pso->Finalize();

	// Create constant buffers
	m_perViewConstantBuffer = make_shared<ConstantBuffer>();
	m_perViewConstantBuffer->Create(sizeof(PerViewConstants), Usage::Dynamic);
	m_perObjectConstantBuffer = make_shared<ConstantBuffer>();
	m_perObjectConstantBuffer->Create(sizeof(PerObjectConstants), Usage::Dynamic);
}