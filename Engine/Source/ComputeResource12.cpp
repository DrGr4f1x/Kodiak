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

#include "ColorBuffer.h"
#include "ComputeKernel12.h"
#include "DepthBuffer.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "RenderThread.h"
#include "Texture.h"


using namespace Kodiak;
using namespace std;


static const D3D12_CPU_DESCRIPTOR_HANDLE g_nullSRV = { ~0ull };
static const D3D12_CPU_DESCRIPTOR_HANDLE g_nullUAV = { ~0ull };


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
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	// Texture must be fully loaded at this point
	assert(texture.IsReady());

	m_cpuHandle = texture.GetSRV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void ComputeResource::SetSRVInternal(DepthBuffer& buffer, bool stencil, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	m_cpuHandle = stencil ? buffer.GetStencilSRV() : buffer.GetDepthSRV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void ComputeResource::SetSRVInternal(ColorBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	m_cpuHandle = buffer.GetSRV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void ComputeResource::SetSRVInternal(GpuBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"ComputeResource is bound to a UAV, but an SRV is being assigned.");
	}

	m_cpuHandle = buffer.GetSRV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void ComputeResource::SetUAVInternal(ColorBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	m_cpuHandle = buffer.GetUAV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void ComputeResource::SetUAVInternal(GpuBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	m_cpuHandle = buffer.GetUAV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceSRV<1>& resource, uint32_t destCpuHandleSlot)
{
	m_type = resource.type;
	m_dimension = resource.dimension;

	m_binding = destCpuHandleSlot;

	_ReadWriteBarrier();

	m_renderThreadData = computeData;

	DispatchToRenderThread(m_cpuHandle, false);
}


void ComputeResource::CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData,
	const ShaderReflection::ResourceUAV<1>& resource, uint32_t destCpuHandleSlot)
{
	m_type = resource.type;

	m_binding = destCpuHandleSlot;

	_ReadWriteBarrier();

	m_renderThreadData = computeData;

	DispatchToRenderThread(m_cpuHandle, false);
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