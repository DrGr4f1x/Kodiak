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
#include "IndexBuffer.h"
#include "PipelineState.h"
#include "Profile.h"
#include "Model.h"
#include "RenderEnums.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "VertexBuffer.h"

#if defined(DX12)
#include "RootSignature12.h"
#endif

#include <ppltasks.h>

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


void Scene::AddStaticModel(shared_ptr<StaticModel> model)
{
	using namespace concurrency;
	
	if (!model->m_renderThreadData)
	{
		auto modelData = make_shared<RenderThread::StaticModelData>();
		model->m_renderThreadData = modelData;
		modelData->matrix = model->m_matrix;

		modelData->scenes.push_back(this);

		auto prepareModelTask = create_task([this, model]()
		{
			auto modelData = model->m_renderThreadData;

			std::vector<concurrency::task<void>> tasks;

			// Determine the maximum number of tasks we need & pre-allocate memory
			uint32_t maxTasks = 0;
			const auto numMeshes = model->GetNumMeshes();
			for (size_t i = 0; i < numMeshes; ++i)
			{
				maxTasks += 2 * static_cast<uint32_t>(model->m_meshes[i].GetNumMeshParts());
			}

			tasks.reserve(maxTasks);

			modelData->meshes.reserve(numMeshes);

			std::vector<shared_ptr<VertexBuffer>> uniqueVBuffers(maxTasks / 2);
			std::vector<shared_ptr<IndexBuffer>> uniqueIBuffers(maxTasks / 2);

			// Construct the render thread copy of meshes and mesh parts
			for (size_t i = 0; i < numMeshes; ++i)
			{
				StaticMeshData meshData;
				XMStoreFloat4x4(&meshData.matrix, XMMatrixIdentity());
				
				const auto& mesh = model->m_meshes[i];
				const auto numParts = mesh.GetNumMeshParts();
							
				for (size_t j = 0; j < numParts; ++j)
				{
					const auto& meshPart = mesh.m_meshParts[j];

					StaticMeshPartData meshPartData;

					// Create index buffer
					meshPartData.indexBuffer = IndexBuffer::Create(meshPart.indexData, Usage::Immutable);

					// Track unique index buffers for this model
					if (end(uniqueIBuffers) == find(begin(uniqueIBuffers), end(uniqueIBuffers), meshPartData.indexBuffer))
					{
						tasks.emplace_back(meshPartData.indexBuffer->loadTask);
						uniqueIBuffers.emplace_back(meshPartData.indexBuffer);
					}

					// Create vertex buffer
					meshPartData.vertexBuffer = VertexBuffer::Create(meshPart.vertexData, Usage::Immutable);

					// Track unique vertex buffers for this model
					if (end(uniqueVBuffers) == find(begin(uniqueVBuffers), end(uniqueVBuffers), meshPartData.vertexBuffer))
					{
						tasks.emplace_back(meshPartData.vertexBuffer->loadTask);
						uniqueVBuffers.emplace_back(meshPartData.vertexBuffer);
					}

					// Fill in misc data for the mesh part draw-call
					meshPartData.topology = meshPart.topology;
					meshPartData.indexCount = meshPart.indexCount;
					meshPartData.baseVertexOffset = meshPart.baseVertexOffset;
					meshPartData.startIndex = meshPart.startIndex;

					meshData.meshParts.emplace_back(meshPartData);
				}

				modelData->meshes.emplace_back(meshData);
			}

			// Wait on all the VB/IB load tasks, then add the model to the scene
			modelData->prepareTask = 
				concurrency::when_all(begin(tasks), end(tasks)).then([this, modelData] { AddStaticModelDeferred(modelData); });
		});
	}
	else
	{
		auto modelData = model->m_renderThreadData;

		// If the render thread data is not part of this scene, add it
		if (end(modelData->scenes) == find(begin(modelData->scenes), end(modelData->scenes), this))
		{
			modelData->scenes.push_back(this);

			modelData->prepareTask.then([this, modelData] { AddStaticModelDeferred(modelData); });
		}
	}
}


void Scene::Update(GraphicsCommandList& commandList)
{
	PROFILE(scene_Update);

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
	PROFILE(scene_Render);

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


void Scene::AddStaticModelDeferred(shared_ptr<RenderThread::StaticModelData> model)
{
	m_deferredAddModels.push(model);
}