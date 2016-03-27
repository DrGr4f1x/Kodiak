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

#include "Material12.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "Texture12.h"


using namespace Kodiak;
using namespace std;


namespace
{

void SetResourceRenderThread(std::shared_ptr<RenderThread::MaterialResourceData> renderThreadData, std::shared_ptr<Texture> texture)
{
	Renderer::GetInstance().EnqueueTask([renderThreadData, texture](RenderTaskEnvironment& rte)
	{
		if (texture)
		{
			renderThreadData->SetResource(texture->GetSRV());
		}
		else
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle(D3D12_DEFAULT);
			renderThreadData->SetResource(handle);
		}
	});
}

} // anonymous namespace


MaterialResource::MaterialResource(const string& name)
	: m_name(name)
	, m_type(ShaderResourceType::Unsupported)
	, m_dimension(ShaderResourceDimension::Unsupported)
	, m_texture(nullptr)
{}


void MaterialResource::SetResource(shared_ptr<Texture> texture)
{
	m_texture = texture;

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;

		if (m_texture)
		{
			if (m_texture->loadTask.is_done())
			{
				SetResourceRenderThread(renderThreadData, texture);
			}
			else
			{
				m_texture->loadTask.then([renderThreadData, texture] { SetResourceRenderThread(renderThreadData, texture); });
			}
		}
		else
		{
			SetResourceRenderThread(renderThreadData, texture);
		}
	}
}


void MaterialResource::CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceSRV<5>& resource)
{
	m_renderThreadData = make_shared<RenderThread::MaterialResourceData>(materialData);

	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& binding = resource.binding[i];
		m_renderThreadData->m_shaderSlots[i].first = binding.tableIndex;
		m_renderThreadData->m_shaderSlots[i].second = binding.tableSlot;
	}

	SetResource(m_texture);
}


RenderThread::MaterialResourceData::MaterialResourceData(shared_ptr<RenderThread::MaterialData> materialData)
	: m_materialData(materialData)
{
	m_srv.ptr = ~0ull;
}


void RenderThread::MaterialResourceData::SetResource(D3D12_CPU_DESCRIPTOR_HANDLE srv)
{
	if (auto materialData = m_materialData.lock())
	{
		if (m_srv.ptr != srv.ptr)
		{
			m_srv = srv;

			assert(m_srv.ptr);

			for (uint32_t i = 0; i < 5; ++i)
			{
				if (m_shaderSlots[i].first != kInvalid)
				{
					materialData->cpuHandles[m_shaderSlots[i].first] = m_srv;
				}
			}
		}
	}
}