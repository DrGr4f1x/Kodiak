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
#include "ColorBuffer.h"
#include "CommandList.h"
#include "ConstantBuffer.h"
#include "DeviceManager.h"
#include "Format.h"
#include "IndexBuffer.h"
#include "PipelineState.h"
#include "Profile.h"
#include "Material.h"
#include "Model.h"
#include "Renderer.h"
#include "RenderEnums.h"
#include "RenderPass.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "ShadowBuffer.h"
#include "ShadowCamera.h"
#include "VertexBuffer.h"

#if defined(DX12)
#include "RootSignature12.h"
#endif

#include <ppltasks.h>

using namespace Kodiak;
using namespace std;
using namespace Math;
using namespace RenderThread;


Scene::Scene()
#if DX11
	: SsaoFullscreen(m_ssaoFullscreen)
#endif
{
	Initialize();
}


void Scene::AddStaticModel(shared_ptr<StaticModel> model)
{
	auto thisScene = shared_from_this();
	auto localModel = model;
	
	Renderer::GetInstance().EnqueueTask([localModel, thisScene](RenderTaskEnvironment& rte)
	{
		thisScene->AddStaticModelDeferred(localModel->m_renderThreadData);
	});
}


void Scene::Update(GraphicsCommandList& commandList)
{
	PROFILE_BEGIN(itt_scene_update);

	// Update per-view constants
	auto cameraProxy = m_camera->GetProxy();
	auto shadowCameraProxy = m_shadowCamera->GetProxy();

	m_perViewConstants.viewProjection = cameraProxy->Base.ViewProjMatrix;
	m_perViewConstants.modelToShadow = shadowCameraProxy->ShadowMatrix;
	m_perViewConstants.viewPosition = cameraProxy->Base.Position;

	auto perViewData = commandList.MapConstants(*m_perViewConstantBuffer);
	memcpy(perViewData, &m_perViewConstants, sizeof(PerViewConstants));
	commandList.UnmapConstants(*m_perViewConstantBuffer);

	// Visit static models
	// TODO: go wide, update 32 per task or something
	for (auto& model : m_staticModels)
	{
		model->UpdateConstants(commandList);

		// Visit meshes
		for (const auto& mesh : model->meshes)
		{
			// Visit mesh parts
			for (const auto& meshPart : mesh->meshParts)
			{
				meshPart.material->Update(commandList);
			}
		}
	}
	PROFILE_END();
}



void Scene::Render(shared_ptr<RenderPass> renderPass, GraphicsCommandList& commandList)
{
	commandList.PIXBeginEvent("Bind sampler states");
	BindSamplerStates(commandList);
	commandList.PIXEndEvent();

	commandList.PIXBeginEvent(renderPass->GetName());
	// Visit models
	for (auto& model : m_staticModels)
	{
		PROFILE_BEGIN(itt_draw_model);
		// Visit meshes
		for (const auto& mesh : model->meshes)
		{
			PROFILE_BEGIN(itt_draw_mesh);
			// Visit mesh parts
			for (const auto& meshPart : mesh->meshParts)
			{
				if (meshPart.material->renderPass == renderPass && meshPart.material->IsReady())
				{
					meshPart.material->Commit(commandList);

					// TODO this is dumb, figure out a better way to bind per-view and per-object constants.  Maybe through material?
#if defined(DX12)
					commandList.SetConstantBuffer(0, *m_perViewConstantBuffer);
					commandList.SetConstantBuffer(1, *mesh->perObjectConstants);
#elif defined(DX11)
					commandList.SetVertexShaderConstants(0, *m_perViewConstantBuffer);
					commandList.SetVertexShaderConstants(1, *mesh->perObjectConstants);

					// TODO bad hack, figure out a different way to handle global textures for a render pass
					if (m_ssaoFullscreen)
					{
						commandList.SetPixelShaderResource(3, m_ssaoFullscreen->GetSRV());
					}
					commandList.SetPixelShaderResource(4, m_shadowBuffer->GetSRV());
#endif
					
					commandList.SetVertexBuffer(0, *meshPart.vertexBuffer);
					commandList.SetIndexBuffer(*meshPart.indexBuffer);
					commandList.SetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)meshPart.topology);

					commandList.DrawIndexed(meshPart.indexCount, meshPart.startIndex, meshPart.baseVertexOffset);
				}
			}
			PROFILE_END();
		}
		PROFILE_END();
	}

#if DX11
	commandList.SetPixelShaderResource(3, nullptr);
	commandList.SetPixelShaderResource(4, nullptr);
#endif

	commandList.PIXEndEvent();
}


void Scene::RenderShadows(shared_ptr<RenderPass> renderPass, GraphicsCommandList& commandList)
{
	// Update per-view constants
	auto cameraProxy = m_camera->GetProxy();
	auto shadowCameraProxy = m_shadowCamera->GetProxy();

	m_perViewConstants.viewProjection = shadowCameraProxy->Base.ViewProjMatrix;
	m_perViewConstants.viewPosition = cameraProxy->Base.Position;

	auto perViewData = commandList.MapConstants(*m_perViewConstantBuffer);
	memcpy(perViewData, &m_perViewConstants, sizeof(PerViewConstants));
	commandList.UnmapConstants(*m_perViewConstantBuffer);

	commandList.PIXBeginEvent("Bind sampler states");
	BindSamplerStates(commandList);
	commandList.PIXEndEvent();

	commandList.PIXBeginEvent(renderPass->GetName());
	// Visit models
	for (auto& model : m_staticModels)
	{
		PROFILE_BEGIN(itt_draw_model);
		// Visit meshes
		for (const auto& mesh : model->meshes)
		{
			PROFILE_BEGIN(itt_draw_mesh);
			// Visit mesh parts
			for (const auto& meshPart : mesh->meshParts)
			{
				if (meshPart.material->renderPass == renderPass && meshPart.material->IsReady())
				{
					meshPart.material->Commit(commandList);

					// TODO this is dumb, figure out a better way to bind per-view and per-object constants.  Maybe through material?
#if defined(DX12)
					commandList.SetConstantBuffer(0, *m_perViewConstantBuffer);
					commandList.SetConstantBuffer(1, *mesh->perObjectConstants);
#elif defined(DX11)
					commandList.SetVertexShaderConstants(0, *m_perViewConstantBuffer);
					commandList.SetVertexShaderConstants(1, *mesh->perObjectConstants);
					commandList.SetPixelShaderResource(3, m_ssaoFullscreen->GetSRV());
#endif

					commandList.SetVertexBuffer(0, *meshPart.vertexBuffer);
					commandList.SetIndexBuffer(*meshPart.indexBuffer);
					commandList.SetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)meshPart.topology);

					commandList.DrawIndexed(meshPart.indexCount, meshPart.startIndex, meshPart.baseVertexOffset);
				}
			}
			PROFILE_END();
		}
		PROFILE_END();
	}

	// TODO hack
#if DX11
	commandList.SetPixelShaderResource(3, nullptr);
	commandList.SetPixelShaderResource(4, nullptr);
#endif

	commandList.PIXEndEvent();
}


void Scene::SetCamera(shared_ptr<Kodiak::Camera> camera)
{
	auto thisScene = shared_from_this();
	Renderer::GetInstance().EnqueueTask([thisScene, camera](RenderTaskEnvironment& rte) { thisScene->SetCameraDeferred(camera); });
}


void Scene::SetShadowBuffer(shared_ptr<ShadowBuffer> buffer)
{
	m_shadowBuffer = buffer;
}


void Scene::SetShadowCamera(shared_ptr<ShadowCamera> camera)
{
	m_shadowCamera = camera;
}


void Scene::Initialize()
{
	// Create per-view constant buffer
	m_perViewConstantBuffer = make_shared<ConstantBuffer>();
	m_perViewConstantBuffer->Create(sizeof(PerViewConstants), Usage::Dynamic);

	// TODO: Remove this and handle sampler state in a non-stupid way
#if defined(DX11)
	auto samplerDesc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8;
	
	ThrowIfFailed(g_device->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf()));

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

	ThrowIfFailed(g_device->CreateSamplerState(&samplerDesc, m_shadowSamplerState.GetAddressOf()));
#endif
}


void Scene::SetCameraDeferred(shared_ptr<Kodiak::Camera> camera)
{
	// TODO: Not threadsafe!!!
	m_camera = camera;
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


void Scene::BindSamplerStates(GraphicsCommandList& commandList)
{
#if defined(DX11)
	commandList.SetPixelShaderSampler(0, m_samplerState.Get());
	commandList.SetPixelShaderSampler(1, m_shadowSamplerState.Get());
#endif
}