// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "MaterialResource11.h"

#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GpuBuffer11.h"
#include "Material11.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "RenderThread.h"
#include "Texture.h"


using namespace Kodiak;
using namespace std;


MaterialResource::MaterialResource(const string& name)
	: m_name(name)
	, m_type(ShaderResourceType::Unsupported)
	, m_dimension(ShaderResourceDimension::Unsupported)
{}


void MaterialResource::SetSRVInternal(Texture& texture, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
	}

	// Texture must be fully loaded at this point
	assert(texture.IsReady());

	SetCachedResources(texture.GetSRV(), nullptr);

	_ReadWriteBarrier();

	DispatchToRenderThread(m_srv.Get(), immediate);
}


void MaterialResource::SetSRVInternal(DepthBuffer& buffer, bool stencil, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(stencil ? buffer.GetStencilSRV() : buffer.GetDepthSRV(), nullptr);

	_ReadWriteBarrier();

	DispatchToRenderThread(m_srv.Get(), immediate);
}


void MaterialResource::SetSRVInternal(ColorBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(buffer.GetSRV(), nullptr);

	_ReadWriteBarrier();

	DispatchToRenderThread(m_srv.Get(), immediate);
}


void MaterialResource::SetSRVInternal(GpuBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(buffer.GetSRV(), nullptr);

	_ReadWriteBarrier();

	DispatchToRenderThread(m_srv.Get(), immediate);
}


void MaterialResource::SetUAVInternal(ColorBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, buffer.GetUAV());

	_ReadWriteBarrier();

	DispatchToRenderThread(m_uav.Get(), immediate);
}


void MaterialResource::SetUAVInternal(GpuBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, buffer.GetUAV());
	
	_ReadWriteBarrier();

	DispatchToRenderThread(m_uav.Get(), immediate);
}


void MaterialResource::CreateRenderThreadData(shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceSRV<5>& resource)
{
	m_type = resource.type;
	m_dimension = resource.dimension;

	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& binding = resource.binding[i];
		m_shaderSlots[i].first = binding.tableIndex;
		m_shaderSlots[i].second = binding.tableSlot;
	}

	_ReadWriteBarrier();

	m_renderThreadData = materialData;

	DispatchToRenderThread(m_srv.Get(), false);
}


void MaterialResource::CreateRenderThreadData(shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceUAV<5>& resource)
{
	m_type = resource.type;

	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& binding = resource.binding[i];
		m_shaderSlots[i].first = binding.tableIndex;
		m_shaderSlots[i].second = binding.tableSlot;
	}

	_ReadWriteBarrier();

	m_renderThreadData = materialData;

	DispatchToRenderThread(m_uav.Get(), false);
}


void MaterialResource::UpdateResourceOnRenderThread(RenderThread::MaterialData* materialData, ID3D11ShaderResourceView* srv)
{
	// Loop over the shader stages
	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& range = m_shaderSlots[i];
		if (range.first != kInvalid)
		{
			materialData->srvTables[i].layouts[range.first].resources[range.second] = srv;
		}
	}
}


void MaterialResource::UpdateResourceOnRenderThread(RenderThread::MaterialData* materialData, ID3D11UnorderedAccessView* uav)
{
	// Loop over the shader stages
	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& range = m_shaderSlots[i];
		if (range.first != kInvalid)
		{
			materialData->uavTables[i].layouts[range.first].resources[range.second] = uav;
		}
	}
}


void MaterialResource::DispatchToRenderThread(ID3D11ShaderResourceView* srv, bool immediate)
{
	if (auto renderThreadData = m_renderThreadData.lock())
	{
		if (immediate)
		{
			UpdateResourceOnRenderThread(renderThreadData.get(), srv);
		}
		else
		{
			// Locals for lambda capture
			auto thisResource = shared_from_this();
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> thisSRV = srv;

			EnqueueRenderCommand([renderThreadData, thisResource, thisSRV]()
			{
				thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), thisSRV.Get());
			});
		}
	}
}


void MaterialResource::DispatchToRenderThread(ID3D11UnorderedAccessView* uav, bool immediate)
{
	if (auto renderThreadData = m_renderThreadData.lock())
	{
		if (immediate)
		{
			UpdateResourceOnRenderThread(renderThreadData.get(), uav);
		}
		else
		{
			// Locals for lambda capture
			auto thisResource = shared_from_this();
			Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> thisUAV = uav;

			EnqueueRenderCommand([renderThreadData, thisResource, thisUAV]()
			{
				thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), thisUAV.Get());
			});
		}
	}
}