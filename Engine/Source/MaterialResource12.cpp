// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "MaterialResource12.h"

#include "ColorBuffer12.h"
#include "DepthBuffer12.h"
#include "Material12.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "Texture12.h"


using namespace Kodiak;
using namespace std;


MaterialResource::MaterialResource(const string& name)
	: m_name(name)
	, m_type(ShaderResourceType::Unsupported)
	, m_dimension(ShaderResourceDimension::Unsupported)
	, m_texture(nullptr)
{}


void MaterialResource::SetTexture(shared_ptr<Texture> texture)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type == ShaderResourceType::Texture || m_type == ShaderResourceType::TBuffer,
			"MaterialResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
	}

	m_texture = texture;
	m_colorBuffer = nullptr;
	m_depthBuffer = nullptr;

	if (auto renderThreadData = m_renderThreadData.lock())
	{
		// Locals for lambda capture
		auto thisResource = shared_from_this();
		shared_ptr<Texture> thisTexture = m_texture;

		if (thisTexture && !thisTexture->loadTask.is_done())
		{
			// Wait until the texture loads before updating the material on the render thread
			m_texture->loadTask.then([renderThreadData, thisResource, thisTexture]
			{
				// Update material on render thread
				Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisTexture](RenderTaskEnvironment& rte)
				{
					auto srv = thisTexture->GetSRV();
					thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
				});
			});
		}
		else
		{
			// Texture is either null or fully loaded.  Either way, we can update the material on the render thread now
			Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisTexture](RenderTaskEnvironment& rte)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE srv;
				if (thisTexture)
				{
					// TODO properly handle null srv here
					srv = thisTexture->GetSRV();
				}
				thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
			});
		}
	}
}


void MaterialResource::SetSRV(shared_ptr<DepthBuffer> buffer, bool stencil)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type != ShaderResourceType::Texture && m_type != ShaderResourceType::TBuffer,
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	m_depthBuffer = buffer;
	m_stencil = stencil;
	m_texture = nullptr;
	m_colorBuffer = nullptr;

	if (auto renderThreadData = m_renderThreadData.lock())
	{
		// Locals for lambda capture
		auto thisResource = shared_from_this();
		auto thisBuffer = m_depthBuffer;
		auto thisStencil = m_stencil;

		// Texture is either null or fully loaded.  Either way, we can update the material on the render thread now
		Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisBuffer, thisStencil](RenderTaskEnvironment& rte)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE srv(D3D12_DEFAULT);
			if (thisBuffer)
			{
				// TODO properly handle null SRV here
				srv = thisStencil ? thisBuffer->GetStencilSRV() : thisBuffer->GetDepthSRV();
			}
			thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
		});
	}
}


void MaterialResource::SetSRV(shared_ptr<ColorBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type != ShaderResourceType::Texture && m_type != ShaderResourceType::TBuffer,
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	m_colorBuffer = buffer;
	m_texture = nullptr;
	m_depthBuffer = nullptr;

	if (auto renderThreadData = m_renderThreadData.lock())
	{
		// Locals for lambda capture
		auto thisResource = shared_from_this();
		auto thisBuffer = m_colorBuffer;

		// Texture is either null or fully loaded.  Either way, we can update the material on the render thread now
		Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisBuffer](RenderTaskEnvironment& rte)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE srv(D3D12_DEFAULT);
			if (thisBuffer)
			{
				// TODO properly handle null SRV here
				srv = thisBuffer->GetSRV();
			}
			thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
		});
	}
}


void MaterialResource::SetUAV(shared_ptr<ColorBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type != ShaderResourceType::Texture && m_type != ShaderResourceType::TBuffer,
			"MaterialResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
	}

	m_colorBuffer = buffer;
	m_texture = nullptr;
	m_depthBuffer = nullptr;

	if (auto renderThreadData = m_renderThreadData.lock())
	{
		// Locals for lambda capture
		auto thisResource = shared_from_this();
		shared_ptr<ColorBuffer> thisBuffer = m_colorBuffer;

		// Texture is either null or fully loaded.  Either way, we can update the material on the render thread now
		Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisBuffer](RenderTaskEnvironment& rte)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE uav(D3D12_DEFAULT);
			if (thisBuffer)
			{
				// TODO properly handle null UAV here
				uav = thisBuffer->GetUAV();
			}
			thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), uav);
		});
	}
}


void MaterialResource::CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceSRV<5>& resource)
{
	m_renderThreadData = materialData;

	m_type = resource.type;
	m_dimension = resource.dimension;

	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& binding = resource.binding[i];
		m_shaderSlots[i].first = binding.tableIndex;
		m_shaderSlots[i].second = binding.tableSlot;
	}

	if (m_texture)
	{
		SetTexture(m_texture);
	}
	else if (m_colorBuffer)
	{
		SetSRV(m_colorBuffer);
	}
	else if (m_depthBuffer)
	{
		SetSRV(m_depthBuffer, m_stencil);
	}
}


void MaterialResource::CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceUAV<5>& resource)
{
	m_renderThreadData = materialData;

	m_type = resource.type;
	
	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& binding = resource.binding[i];
		m_shaderSlots[i].first = binding.tableIndex;
		m_shaderSlots[i].second = binding.tableSlot;
	}

	SetUAV(m_colorBuffer);
}


void MaterialResource::UpdateResourceOnRenderThread(RenderThread::MaterialData* materialData, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
{
	// Loop over the shader stages
	for (uint32_t i = 0; i < 5; ++i)
	{
		if (m_shaderSlots[i].first != kInvalid)
		{
			materialData->cpuHandles[m_shaderSlots[i].first] = cpuHandle;
		}
	}
}