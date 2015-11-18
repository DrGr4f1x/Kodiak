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

#include "CommandList.h"
#include "ConstantBuffer.h"
#include "PipelineState.h"
#include "Model.h"
#include "RenderEnums.h"
#include "ShaderManager.h"


using namespace Kodiak;
using namespace std;


Scene::Scene(uint32_t width, uint32_t height) : m_width(width), m_height(height)
{
	Initialize();
}


void Scene::AddModel(shared_ptr<Model> model)
{
	m_models.push_back(model);
}


void Scene::Render(GraphicsCommandList& commandList)
{
	// Update per-view constants
	auto perViewData = commandList.MapConstants(*m_perViewConstantBuffer);
	memcpy(perViewData, &m_perViewConstants, sizeof(PerViewConstants));
	commandList.UnmapConstants(*m_perViewConstantBuffer);

	// Update per-object constants
	auto perObjectData = commandList.MapConstants(*m_perObjectConstantBuffer);
	memcpy(perObjectData, &m_perObjectConstants, sizeof(PerObjectConstants));
	commandList.UnmapConstants(*m_perObjectConstantBuffer);

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
				commandList.SetVertexShaderConstants(0, *m_perViewConstantBuffer);
				commandList.SetVertexShaderConstants(1, *m_perObjectConstantBuffer);

				commandList.SetVertexBuffer(0, *meshPart.m_vertexBuffer);
				commandList.SetIndexBuffer(*meshPart.m_indexBuffer);
				commandList.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				commandList.DrawIndexed(meshPart.m_indexCount, meshPart.m_startIndex, meshPart.m_baseVertexOffset);
			}
		}
	}
}


void Scene::Initialize()
{
	using namespace DirectX;

	float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f);

	XMStoreFloat4x4(&m_perViewConstants.projection,	XMMatrixTranspose(perspectiveMatrix));

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_perViewConstants.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	XMStoreFloat4x4(&m_perObjectConstants.model, XMMatrixIdentity());

	// Default blend state - no blend
	BlendStateDesc defaultBlendState;

	// Depth-stencil state - normal depth testing
	DepthStencilStateDesc depthStencilState(true, true);

	// Rasterizer state - two-sided
	RasterizerStateDesc rasterizerState(CullMode::Back, FillMode::Solid);

	// Load shaders
	auto vs = ShaderManager::GetInstance().LoadVertexShader("Engine", "SimpleVertexShader.cso");
	auto ps = ShaderManager::GetInstance().LoadPixelShader("Engine", "SimplePixelShader.cso");
	(vs->loadTask && ps->loadTask).wait();

	// Configure PSO
	m_pso = make_shared<GraphicsPSO>();
	m_pso->SetBlendState(defaultBlendState);
	m_pso->SetRasterizerState(rasterizerState);
	m_pso->SetDepthStencilState(depthStencilState);
	m_pso->SetInputLayout(*vs->GetInputLayout());
	m_pso->SetVertexShader(vs.get());
	m_pso->SetPixelShader(ps.get());

	m_pso->Finalize();

	// Create constant buffers
	m_perViewConstantBuffer = make_shared<ConstantBuffer>();
	m_perViewConstantBuffer->Create(sizeof(PerViewConstants), Usage::Dynamic);
	m_perObjectConstantBuffer = make_shared<ConstantBuffer>();
	m_perObjectConstantBuffer->Create(sizeof(PerObjectConstants), Usage::Dynamic);
}