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

#include "ColorBuffer11.h"
#include "DepthBuffer.h"
#include "GpuBuffer11.h"
#include "Material11.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "RenderThread.h"
#include "Texture11.h"


using namespace Kodiak;
using namespace std;


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
			"MaterialResource is bound to a UAV, but a Texture is being assigned (should be a ColorBuffer).");
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
			DispatchToRenderThreadNoLock(renderThreadData, texture ? texture->GetSRV() : nullptr);
		}
	}
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
		DispatchToRenderThread((ID3D11ShaderResourceView*)nullptr);
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

	DispatchToRenderThread(buffer ? buffer->GetSRV() : nullptr);
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

	DispatchToRenderThread(buffer ? buffer->GetSRV() : nullptr);
}


void MaterialResource::SetUAV(shared_ptr<ColorBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, buffer, nullptr, nullptr, false);

	_ReadWriteBarrier();

	DispatchToRenderThread(buffer ? buffer->GetUAV() : nullptr);
}


void MaterialResource::SetUAV(shared_ptr<GpuBuffer> buffer)
{
	// Validate type
	if (m_type != ShaderResourceType::Unsupported)
	{
		assert_msg(IsUAVType(m_type),
			"ComputeResource is bound to an SRV, but a UAV is being assigned.");
	}

	SetCachedResources(nullptr, nullptr, nullptr, buffer, false);
	
	_ReadWriteBarrier();

	DispatchToRenderThread(buffer ? buffer->GetUAV() : nullptr);
}


static void whoa() {}
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

	if (m_texture)
	{
		SetSRV(m_texture);
		return;
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
	whoa();
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

	if (m_colorBuffer)
	{
		SetUAV(m_colorBuffer);
	}
	else if (m_gpuBuffer)
	{
		SetUAV(m_gpuBuffer);
	}
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


void MaterialResource::DispatchToRenderThread(ID3D11ShaderResourceView* srv)
{
	if (auto renderThreadData = m_renderThreadData.lock())
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


void MaterialResource::DispatchToRenderThread(ID3D11UnorderedAccessView* uav)
{
	if (auto renderThreadData = m_renderThreadData.lock())
	{
		// Locals for lambda capture
		auto thisResource = shared_from_this();
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> thisUAV = uav;

		Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisUAV](RenderTaskEnvironment& rte)
		{
			thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), thisUAV.Get());
		});
	}
}


void MaterialResource::DispatchToRenderThreadNoLock(shared_ptr<RenderThread::MaterialData> materialData, ID3D11ShaderResourceView* srv)
{
	// Locals for lambda capture
	auto thisResource = shared_from_this();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> thisSRV = srv;

	Renderer::GetInstance().EnqueueTask([materialData, thisResource, thisSRV](RenderTaskEnvironment& rte)
	{
		thisResource->UpdateResourceOnRenderThread(materialData.get(), thisSRV.Get());
	});
}