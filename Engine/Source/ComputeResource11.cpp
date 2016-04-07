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


void ComputeResource::SetUAV(shared_ptr<ColorBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(m_type != ShaderResourceType::Texture && m_type != ShaderResourceType::TBuffer,
			"ComputeResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
	}

	m_buffer = buffer;

	if (auto renderThreadData = m_renderThreadData.lock())
	{
		// Locals for lambda capture
		auto thisResource = shared_from_this();
		shared_ptr<ColorBuffer> thisBuffer = m_buffer;

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
	assert_msg(!m_buffer, "ComputeResource is bound to an SRV, but has a ColorBuffer assigned (should be a Texture).");

	m_renderThreadData = computeData;

	m_type = resource.type;
	m_dimension = resource.dimension;

	m_bindingTable = resource.binding[0].tableIndex;
	m_bindingSlot = resource.binding[0].tableSlot;

	SetTexture(m_texture);
}


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceUAV<1>& resource)
{
	assert_msg(!m_buffer, "ComputeResource is bound to a UAV, but has a Texture assigned (should be a ColorBuffer).");

	m_renderThreadData = computeData;

	m_type = resource.type;

	m_bindingTable = resource.binding[0].tableIndex;
	m_bindingSlot = resource.binding[0].tableSlot;

	SetUAV(m_buffer);
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