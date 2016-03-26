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

#include "Material11.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "Texture11.h"


using namespace Kodiak;
using namespace std;


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
			m_texture->loadTask.then([renderThreadData, texture]
			{
				Renderer::GetInstance().EnqueueTask([renderThreadData, texture](RenderTaskEnvironment& rte)
				{
					if (texture)
					{
						renderThreadData->SetResource(texture->GetSRV());
					}
					else
					{
						renderThreadData->SetResource(nullptr);
					}
				});
			});
		}
		else
		{
			Renderer::GetInstance().EnqueueTask([renderThreadData, texture](RenderTaskEnvironment& rte)
			{
				if (texture)
				{
					renderThreadData->SetResource(texture->GetSRV());
				}
				else
				{
					renderThreadData->SetResource(nullptr);
				}
			});
		}
	}
}


void MaterialResource::CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const Effect::ResourceSRV& resource)
{
	m_renderThreadData = make_shared<RenderThread::MaterialResourceData>(materialData);

	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& binding = resource.bindings[i];
		m_renderThreadData->m_shaderSlots[i].first = binding.tableIndex;
		m_renderThreadData->m_shaderSlots[i].second = binding.tableSlot;
	}

	SetResource(m_texture);
}


RenderThread::MaterialResourceData::MaterialResourceData(shared_ptr<RenderThread::MaterialData> materialData)
	: m_materialData(materialData)
{}


void RenderThread::MaterialResourceData::SetResource(ID3D11ShaderResourceView* srv)
{
	if (auto materialData = m_materialData.lock())
	{
		if (m_srv.Get() != srv)
		{
			m_srv = srv;

			for (uint32_t i = 0; i < 5; ++i)
			{
				const auto& range = m_shaderSlots[i];
				if (range.first != kInvalid)
				{
					materialData->srvTables[i].layouts[range.first].resources[range.second] = srv;
				}
			}
		}
	}
}