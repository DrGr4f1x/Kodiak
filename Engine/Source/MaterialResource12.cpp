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


void MaterialResource::SetSRV(shared_ptr<Texture> texture)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
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
				thisResource->DispatchToRenderThreadNoLock(renderThreadData, texture->GetSRV());
			});
		}
		else
		{
			DispatchToRenderThreadNoLock(renderThreadData, texture ? texture->GetSRV() : g_nullSRV );
		}
	}
}


void MaterialResource::SetSRV(shared_ptr<ColorBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(nullptr, buffer, nullptr, nullptr, false);

	_ReadWriteBarrier();

	DispatchToRenderThread(buffer ? buffer->GetSRV() : g_nullSRV);
}


void MaterialResource::SetSRV(shared_ptr<DepthBuffer> buffer, bool stencil)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(nullptr, nullptr, buffer, nullptr, stencil);

	_ReadWriteBarrier();

	if (buffer)
	{
		DispatchToRenderThread(stencil ? buffer->GetStencilSRV() : buffer->GetDepthSRV());
	}
	else
	{
		DispatchToRenderThread(g_nullSRV);
	}
}


void MaterialResource::SetSRV(shared_ptr<GpuBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsSRVType(m_type),
			"MaterialResource is bound to a UAV, but an SRV is being assigned.");
	}

	SetCachedResources(nullptr, nullptr, nullptr, buffer, false);

	_ReadWriteBarrier();

	DispatchToRenderThread(buffer ? buffer->GetSRV() : g_nullSRV);
}


void MaterialResource::SetUAV(shared_ptr<ColorBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"MaterialResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, buffer, nullptr, nullptr, false);

	_ReadWriteBarrier();

	DispatchToRenderThread(buffer ? buffer->GetUAV() : g_nullUAV);
}


void MaterialResource::SetUAV(shared_ptr<GpuBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"MaterialResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, nullptr, nullptr, buffer, false);

	_ReadWriteBarrier();

	DispatchToRenderThread(buffer ? buffer->GetUAV() : g_nullUAV);
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

	if (m_colorBuffer)
	{
		SetUAV(m_colorBuffer);
	}
	else if (m_gpuBuffer)
	{
		SetUAV(m_gpuBuffer);
	}
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


void MaterialResource::DispatchToRenderThread(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
{
	if (auto renderThreadData = m_renderThreadData.lock())
	{
		auto thisResource = shared_from_this();
		Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, cpuHandle](RenderTaskEnvironment& rte)
		{
			thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), cpuHandle);
		});
	}
}


void MaterialResource::DispatchToRenderThreadNoLock(shared_ptr<RenderThread::MaterialData> materialData, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
{
	auto thisResource = shared_from_this();
	Renderer::GetInstance().EnqueueTask([materialData, thisResource, cpuHandle](RenderTaskEnvironment& rte)
	{
		thisResource->UpdateResourceOnRenderThread(materialData.get(), cpuHandle);
	});
}