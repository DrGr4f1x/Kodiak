// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ComputeResource11.h"

#include "ColorBuffer11.h"
#include "ComputeKernel11.h"
#include "DepthBuffer11.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "Texture11.h"


using namespace Kodiak;
using namespace std;


ComputeResource::ComputeResource(const string& name)
	: m_name(name)
	, m_type(ShaderResourceType::Unsupported)
	, m_dimension(ShaderResourceDimension::Unsupported)
	, m_texture(nullptr)
{}


void ComputeResource::SetTexture(shared_ptr<Texture> texture)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type == ShaderResourceType::Texture || m_type == ShaderResourceType::TBuffer,
			"ComputeResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
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
			// Wait until the texture loads before updating the compute kernel on the render thread
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
			// Texture is either null or fully loaded.  Either way, we can update the compute kernel on the render thread now
			Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisTexture](RenderTaskEnvironment& rte)
			{
				ID3D11ShaderResourceView* srv{ nullptr };
				if (thisTexture)
				{
					// TODO properly handle null SRV here
					srv = thisTexture->GetSRV();
				}
				thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
			});
		}
	}
}


void ComputeResource::SetSRV(shared_ptr<DepthBuffer> buffer, bool stencil)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type == ShaderResourceType::Texture || m_type == ShaderResourceType::TBuffer,
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
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
			ID3D11ShaderResourceView* srv = nullptr;
			if (thisBuffer)
			{
				// TODO properly handle null SRV here
				srv = thisStencil ? thisBuffer->GetStencilSRV() : thisBuffer->GetDepthSRV();
			}
			thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
		});
	}
}


void ComputeResource::SetSRV(shared_ptr<ColorBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type == ShaderResourceType::Texture || m_type == ShaderResourceType::TBuffer,
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
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
			ID3D11ShaderResourceView* srv = nullptr;
			if (thisBuffer)
			{
				// TODO properly handle null SRV here
				srv = thisBuffer->GetSRV();
			}
			thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
		});
	}
}


void ComputeResource::SetUAV(shared_ptr<ColorBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type != ShaderResourceType::Texture && m_type != ShaderResourceType::TBuffer,
			"ComputeResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
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
			ID3D11UnorderedAccessView* uav{ nullptr };
			if (thisBuffer)
			{
				// TODO properly handle null UAV here
				uav = thisBuffer->GetUAV();
			}
			thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), uav);
		});
	}
}


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceSRV<1>& resource)
{
	m_renderThreadData = computeData;

	m_type = resource.type;
	m_dimension = resource.dimension;

	m_bindingTable = resource.binding[0].tableIndex;
	m_bindingSlot = resource.binding[0].tableSlot;

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


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceUAV<1>& resource)
{
	m_renderThreadData = computeData;

	m_type = resource.type;

	m_bindingTable = resource.binding[0].tableIndex;
	m_bindingSlot = resource.binding[0].tableSlot;

	SetUAV(m_colorBuffer);
}


void ComputeResource::UpdateResourceOnRenderThread(RenderThread::ComputeData* computeData, ID3D11ShaderResourceView* srv)
{
	if (m_bindingTable != kInvalid)
	{
		computeData->srvTables.layouts[m_bindingTable].resources[m_bindingSlot] = srv;
	}
}


void ComputeResource::UpdateResourceOnRenderThread(RenderThread::ComputeData* computeData, ID3D11UnorderedAccessView* uav)
{
	if (m_bindingTable != kInvalid)
	{
		computeData->uavTables.layouts[m_bindingTable].resources[m_bindingSlot] = uav;
	}
}