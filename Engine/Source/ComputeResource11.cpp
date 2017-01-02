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

#include "ColorBuffer.h"
#include "ComputeKernel11.h"
#include "DepthBuffer.h"
#include "GpuBuffer11.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "RenderThread.h"
#include "Texture.h"


using namespace Kodiak;
using namespace std;


ComputeResource::ComputeResource(const string& name)
	: m_name(name)
	, m_type(ShaderResourceType::Unsupported)
	, m_dimension(ShaderResourceDimension::Unsupported)
{}



void ComputeResource::SetSRVInternal(Texture& texture, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
	}

	// Texture must be fully loaded at this point
	assert(texture.IsReady());

	SetCachedResources(texture.GetSRV(), nullptr);

	_ReadWriteBarrier();

	DispatchToRenderThread(m_srv.Get(), immediate);
}


void ComputeResource::SetSRVInternal(DepthBuffer& buffer, bool stencil, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(stencil ? buffer.GetStencilSRV() : buffer.GetDepthSRV(), nullptr);

	_ReadWriteBarrier();

	DispatchToRenderThread(m_srv.Get(), immediate);
}


void ComputeResource::SetSRVInternal(ColorBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(buffer.GetSRV(), nullptr);

	_ReadWriteBarrier();

	DispatchToRenderThread(m_srv.Get(), immediate);
}


void ComputeResource::SetSRVInternal(GpuBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(buffer.GetSRV(), nullptr);

	_ReadWriteBarrier();

	DispatchToRenderThread(m_srv.Get(), immediate);
}


void ComputeResource::SetUAVInternal(ColorBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, buffer.GetUAV());

	_ReadWriteBarrier();

	m_counterInitialValue = 0; // Unused for ColorBuffer UAVs
	DispatchToRenderThread(m_uav.Get(), m_counterInitialValue, immediate);
}


void ComputeResource::SetUAVInternal(GpuBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, buffer.GetUAV());

	_ReadWriteBarrier();

	m_counterInitialValue = buffer.GetCounterInitialValue();

	DispatchToRenderThread(m_uav.Get(), m_counterInitialValue, immediate);
}


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceSRV<1>& resource)
{
	m_type = resource.type;
	m_dimension = resource.dimension;

	m_bindingTable = resource.binding[0].tableIndex;
	m_bindingSlot = resource.binding[0].tableSlot;

	_ReadWriteBarrier();

	m_renderThreadData = computeData;

	DispatchToRenderThread(m_srv.Get(), false);
}


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceUAV<1>& resource)
{
	m_type = resource.type;

	m_bindingTable = resource.binding[0].tableIndex;
	m_bindingSlot = resource.binding[0].tableSlot;

	_ReadWriteBarrier();

	m_renderThreadData = computeData;

	DispatchToRenderThread(m_uav.Get(), m_counterInitialValue, false);
}


void ComputeResource::UpdateResourceOnRenderThread(RenderThread::ComputeData* computeData, ID3D11ShaderResourceView* srv)
{
	if (m_bindingTable != kInvalid)
	{
		computeData->srvTables.layouts[m_bindingTable].resources[m_bindingSlot] = srv;
	}
}


void ComputeResource::UpdateResourceOnRenderThread(RenderThread::ComputeData* computeData, ID3D11UnorderedAccessView* uav, uint32_t counterInitialValue)
{
	if (m_bindingTable != kInvalid)
	{
		computeData->uavTables.layouts[m_bindingTable].resources[m_bindingSlot] = uav;
		computeData->uavTables.layouts[m_bindingTable].counterInitialValues[m_bindingSlot] = counterInitialValue;
	}
}


void ComputeResource::DispatchToRenderThread(ID3D11ShaderResourceView* srv, bool bImmediate)
{
	if (auto renderThreadData = m_renderThreadData.lock())
	{
		if (bImmediate)
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


void ComputeResource::DispatchToRenderThread(ID3D11UnorderedAccessView* uav, uint32_t counterInitialValue, bool bImmediate)
{
	if (auto renderThreadData = m_renderThreadData.lock())
	{
		if (bImmediate)
		{
			UpdateResourceOnRenderThread(renderThreadData.get(), uav, counterInitialValue);
		}
		else
		{
			// Locals for lambda capture
			auto thisResource = shared_from_this();
			Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> thisUAV = uav;

			// Texture is either null or fully loaded.  Either way, we can update the material on the render thread now
			EnqueueRenderCommand([renderThreadData, thisResource, thisUAV, counterInitialValue]()
			{
				thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), thisUAV.Get(), counterInitialValue);
			});
		}
	}
}