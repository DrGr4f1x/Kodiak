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
#include "DepthBuffer.h"
#include "GpuBuffer11.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "RenderThread.h"
#include "Texture11.h"


using namespace Kodiak;
using namespace std;


ComputeResource::ComputeResource(const string& name)
	: m_name(name)
	, m_type(ShaderResourceType::Unsupported)
	, m_dimension(ShaderResourceDimension::Unsupported)
	, m_texture(nullptr)
{}



void ComputeResource::SetSRVInternal(shared_ptr<Texture> texture, bool bImmediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
	}

	SetCachedResources(texture, nullptr, nullptr, nullptr, false);

	_ReadWriteBarrier();

	if (auto renderThreadData = m_renderThreadData.lock())
	{
		if (texture && !texture->loadTask.is_done())
		{
			auto thisResource = shared_from_this();
			// Wait until the texture loads before updating the material on the render thread
			texture->loadTask.then([renderThreadData, thisResource, texture, bImmediate]
			{
				SetThreadRole(ThreadRole::GenericWorker);
				thisResource->DispatchToRenderThreadNoLock(renderThreadData, texture->GetSRV(), false);
			});
		}
		else
		{
			DispatchToRenderThreadNoLock(renderThreadData, texture ? texture->GetSRV() : nullptr, bImmediate);
		}
	}
}


void ComputeResource::SetSRVInternal(shared_ptr<DepthBuffer> buffer, bool stencil, bool bImmediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(nullptr, nullptr, buffer, nullptr, stencil);

	_ReadWriteBarrier();

	if (buffer)
	{
		DispatchToRenderThread(stencil ? buffer->GetStencilSRV() : buffer->GetDepthSRV(), bImmediate);
	}
	else
	{
		DispatchToRenderThread((ID3D11ShaderResourceView*)nullptr, bImmediate);
	}
}


void ComputeResource::SetSRVInternal(shared_ptr<ColorBuffer> buffer, bool bImmediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(nullptr, buffer, nullptr, nullptr, false);

	_ReadWriteBarrier();

	DispatchToRenderThread(buffer ? buffer->GetSRV() : nullptr, bImmediate);
}


void ComputeResource::SetSRVInternal(shared_ptr<GpuBuffer> buffer, bool bImmediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(nullptr, nullptr, nullptr, buffer, false);

	_ReadWriteBarrier();

	DispatchToRenderThread(buffer ? buffer->GetSRV() : nullptr, bImmediate);
}


void ComputeResource::SetUAVInternal(shared_ptr<ColorBuffer> buffer, bool bImmediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, buffer, nullptr, nullptr, false);

	_ReadWriteBarrier();

	const uint32_t counterInitialValue = 0; // Unused for ColorBuffer UAVs
	DispatchToRenderThread(buffer ? buffer->GetUAV() : nullptr, counterInitialValue, bImmediate);
}


void ComputeResource::SetUAVInternal(shared_ptr<GpuBuffer> buffer, bool bImmediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, nullptr, nullptr, buffer, false);

	_ReadWriteBarrier();

	ID3D11UnorderedAccessView* uav = buffer ? buffer->GetUAV() : nullptr;
	uint32_t counterInitialValue = buffer ? buffer->GetCounterInitialValue() : 0;

	DispatchToRenderThread(uav, counterInitialValue, bImmediate);
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

	if (m_texture)
	{
		SetSRV(m_texture);
	}
	else if (m_colorBuffer)
	{
		SetSRV(m_colorBuffer);
	}
	else if (m_depthBuffer)
	{
		SetSRV(m_depthBuffer, m_stencil);
	}
	else if (m_gpuBuffer)
	{
		SetSRV(m_gpuBuffer);
	}
}


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceUAV<1>& resource)
{
	m_type = resource.type;

	m_bindingTable = resource.binding[0].tableIndex;
	m_bindingSlot = resource.binding[0].tableSlot;

	_ReadWriteBarrier();

	m_renderThreadData = computeData;

	if (m_colorBuffer)
	{
		SetUAV(m_colorBuffer);
	}
	else if (m_gpuBuffer)
	{
		SetUAV(m_gpuBuffer);
	}
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

			Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisSRV](RenderTaskEnvironment& rte)
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
			Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisUAV, counterInitialValue](RenderTaskEnvironment& rte)
			{
				thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), thisUAV.Get(), counterInitialValue);
			});
		}
	}
}


void ComputeResource::DispatchToRenderThreadNoLock(shared_ptr<RenderThread::ComputeData> materialData, ID3D11ShaderResourceView* srv, bool bImmediate)
{
	if (bImmediate)
	{
		UpdateResourceOnRenderThread(materialData.get(), srv);
	}
	else
	{
		// Locals for lambda capture
		auto thisResource = shared_from_this();
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> thisSRV = srv;

		Renderer::GetInstance().EnqueueTask([materialData, thisResource, thisSRV](RenderTaskEnvironment& rte)
		{
			thisResource->UpdateResourceOnRenderThread(materialData.get(), thisSRV.Get());
		});
	}
}