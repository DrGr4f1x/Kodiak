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
#include "Renderer.h"
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


void Scene::AddStaticModel(shared_ptr<StaticModel> model)
{
	//Renderer::GetInstance().AddStaticModelToScene(model, shared_from_this());
	auto staticModelData = model->GetRenderThreadData();
	// TODO: get rid of this if-check.  RenderThreadData should never be null, as long as model isn't null.
	if (!staticModelData)
	{
		model->CreateRenderThreadData();
	}
	staticModelData = model->GetRenderThreadData();
	if(staticModelData)
	{
		auto thisScene = shared_from_this();
		staticModelData->prepareTask.then([staticModelData, thisScene]
		{
			Renderer::GetInstance().EnqueueTask([staticModelData, thisScene](RenderTaskEnvironment& rte)
			{
				thisScene->AddStaticModelDeferred(staticModelData);
			});
		});
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

	// Visit static models
	// TODO: go wide, update 32 per task or something
	for (auto& model : m_staticModels)
	{
		if (model->isDirty)
		{
			model->UpdateConstants(commandList);
		}
	}
}


void Scene::Render(GraphicsCommandList& commandList)
{
	PROFILE(scene_Render);

	// Visit models
	for (auto& model : m_staticModels)
	{
		// Visit meshes
		for (const auto& mesh : model->meshes)
		{
			// Visit mesh parts
			for (const auto& meshPart : mesh.meshParts)
			{
				commandList.SetPipelineState(*m_pso);
#if defined(DX12)
				commandList.SetRootSignature(*m_rootSignature);
				commandList.SetConstantBuffer(0, *m_perViewConstantBuffer);
				commandList.SetConstantBuffer(1, *mesh.perObjectConstants);
#elif defined(DX11)
				commandList.SetVertexShaderConstants(0, *m_perViewConstantBuffer);
				commandList.SetVertexShaderConstants(1, *mesh.perObjectConstants);
#endif

				commandList.SetVertexBuffer(0, *meshPart.vertexBuffer);
				commandList.SetIndexBuffer(*meshPart.indexBuffer);
				commandList.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				commandList.DrawIndexed(meshPart.indexCount, meshPart.startIndex, meshPart.baseVertexOffset);
			}
		}
	}
}


void Scene::SetCamera(shared_ptr<Kodiak::Camera> camera)
{
	auto thisScene = shared_from_this();
	Renderer::GetInstance().EnqueueTask([thisScene, camera](RenderTaskEnvironment& rte) { thisScene->SetCameraDeferred(camera); });
}


void Scene::Initialize()
{
	using namespace DirectX;

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
}


void Scene::SetCameraDeferred(shared_ptr<Kodiak::Camera> camera)
{
	// TODO: create Camera proxy in camera constructor
	if (!camera->GetProxy())
	{
		camera->CreateProxy();
	}

	m_camera = camera->GetProxy();
}


void Scene::AddStaticModelDeferred(shared_ptr<RenderThread::StaticModelData> model)
{
	auto it = m_staticModelMap.find(model);

	// Only add the model to the scene once
	if (it == end(m_staticModelMap))
	{
		const auto newIndex = m_staticModels.size();

		// TODO: handle other data lists here, e.g. bounding boxes

		m_staticModels.emplace_back(model);
		m_staticModelMap[model] = newIndex;
	}
}


void Scene::RemoveStaticModelDeferred(shared_ptr<RenderThread::StaticModelData> model)
{
	auto it = m_staticModelMap.find(model);
	if (it != end(m_staticModelMap))
	{
		const auto numStaticModels = m_staticModels.size();
		const auto index = it->second;

		assert(index < numStaticModels);

		// TODO: handle other data lists here, e.g. bounding boxes

		if (numStaticModels > 1)
		{
			// Update the map
			m_staticModelMap[m_staticModels.back()] = index;
			m_staticModelMap.erase(m_staticModels[index]);

			swap(m_staticModels[index], m_staticModels.back());
			m_staticModels.pop_back();
		}
		else
		{
			m_staticModelMap.erase(m_staticModels[0]);
			m_staticModels.pop_back();
		}
	}
}