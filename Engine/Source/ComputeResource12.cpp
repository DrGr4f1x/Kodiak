// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ComputeResource12.h"

#include "ColorBuffer12.h"
#include "ComputeKernel12.h"
#include "DepthBuffer.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "RenderThread.h"
#include "Texture12.h"


using namespace Kodiak;
using namespace std;


static const D3D12_CPU_DESCRIPTOR_HANDLE g_nullSRV = { ~0ull };
static const D3D12_CPU_DESCRIPTOR_HANDLE g_nullUAV = { ~0ull };


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
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(texture, nullptr, nullptr, nullptr, false);

	_ReadWriteBarrier();

	if (auto renderThreadData = m_renderThreadData.lock())
	{
		if (texture && !texture->loadTask.is_done())
		{
			auto thisResource = shared_from_this();
			// Wait until the texture loads before updating the material on the render thread
			texture->loadTask.then([renderThreadData, thisResource, texture]
			{
				SetThreadRole(ThreadRole::GenericWorker);
				thisResource->DispatchToRenderThreadNoLock(renderThreadData, texture->GetSRV(), false);
			});
		}
		else
		{
			DispatchToRenderThreadNoLock(renderThreadData, texture ? texture->GetSRV() : g_nullSRV, bImmediate);
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
		DispatchToRenderThread(g_nullSRV, bImmediate);
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

	DispatchToRenderThread(buffer ? buffer->GetSRV() : g_nullSRV, bImmediate);
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

	DispatchToRenderThread(buffer ? buffer->GetSRV() : g_nullSRV, bImmediate);
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

	DispatchToRenderThread(buffer ? buffer->GetUAV() : g_nullUAV, bImmediate);
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

	DispatchToRenderThread(buffer ? buffer->GetUAV() : g_nullUAV, bImmediate);
}


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceSRV<1>& resource, uint32_t destCpuHandleSlot)
{
	m_type = resource.type;
	m_dimension = resource.dimension;

	m_binding = destCpuHandleSlot;

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
	const ShaderReflection::ResourceUAV<1>& resource, uint32_t destCpuHandleSlot)
{
	m_type = resource.type;

	m_binding = destCpuHandleSlot;

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


void ComputeResource::UpdateResourceOnRenderThread(RenderThread::ComputeData* computeData, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
{
	// Loop over the shader stages
	if (m_binding != kInvalid)
	{
		computeData->cpuHandles[m_binding] = cpuHandle;
	}
}


void ComputeResource::DispatchToRenderThread(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, bool bImmediate)
{
	if (auto renderThreadData = m_renderThreadData.lock())
	{
		if (bImmediate)
		{
			UpdateResourceOnRenderThread(renderThreadData.get(), cpuHandle);
		}
		else
		{
			auto thisResource = shared_from_this();
			Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, cpuHandle](RenderTaskEnvironment& rte)
			{
				thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), cpuHandle);
			});
		}
	}
}


void ComputeResource::DispatchToRenderThreadNoLock(shared_ptr<RenderThread::ComputeData> computeData, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, bool bImmediate)
{
	if (bImmediate)
	{
		UpdateResourceOnRenderThread(computeData.get(), cpuHandle);
	}
	else
	{
		auto thisResource = shared_from_this();
		Renderer::GetInstance().EnqueueTask([computeData, thisResource, cpuHandle](RenderTaskEnvironment& rte)
		{
			thisResource->UpdateResourceOnRenderThread(computeData.get(), cpuHandle);
		});
	}
}