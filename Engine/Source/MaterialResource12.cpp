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
#include "DepthBuffer.h"
#include "GpuBuffer.h"
#include "Material12.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "RenderThread.h"
#include "Texture12.h"


using namespace Kodiak;
using namespace std;


static const D3D12_CPU_DESCRIPTOR_HANDLE g_nullSRV = { ~0ull };
static const D3D12_CPU_DESCRIPTOR_HANDLE g_nullUAV = { ~0ull };


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
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	// Texture must be fully loaded at this point
	assert(texture != false);

	m_cpuHandle = texture.GetSRV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void MaterialResource::SetSRVInternal(ColorBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	m_cpuHandle = buffer.GetSRV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void MaterialResource::SetSRVInternal(DepthBuffer& buffer, bool stencil, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	m_cpuHandle = stencil ? buffer.GetStencilSRV() : buffer.GetDepthSRV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void MaterialResource::SetSRVInternal(GpuBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	m_cpuHandle = buffer.GetSRV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void MaterialResource::SetUAVInternal(ColorBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"MaterialResource is bound to an SRV, but a UAV is being assigned.");
	}

	m_cpuHandle = buffer.GetUAV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void MaterialResource::SetUAVInternal(GpuBuffer& buffer, bool immediate)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"MaterialResource is bound to an SRV, but a UAV is being assigned.");
	}

	m_cpuHandle = buffer.GetUAV();

	_ReadWriteBarrier();

	DispatchToRenderThread(m_cpuHandle, immediate);
}


void MaterialResource::CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceSRV<5>& resource)
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

	DispatchToRenderThread(m_cpuHandle, false);
}


void MaterialResource::CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceUAV<5>& resource)
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

	DispatchToRenderThread(m_cpuHandle, false);
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


void MaterialResource::DispatchToRenderThread(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, bool immediate)
{
	if (auto renderThreadData = m_renderThreadData.lock())
	{
		if(immediate)
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


void MaterialResource::DispatchToRenderThreadNoLock(shared_ptr<RenderThread::MaterialData> materialData, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, bool immediate)
{
	if (immediate)
	{
		UpdateResourceOnRenderThread(materialData.get(), cpuHandle);
	}
	else
	{
		auto thisResource = shared_from_this();
		Renderer::GetInstance().EnqueueTask([materialData, thisResource, cpuHandle](RenderTaskEnvironment& rte)
		{
			thisResource->UpdateResourceOnRenderThread(materialData.get(), cpuHandle);
		});
	}
}