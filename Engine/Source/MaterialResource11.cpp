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

	if (auto renderThreadData = m_renderThreadData.lock())
	{
		// Locals for lambda capture
		auto thisResource = shared_from_this();
		shared_ptr<Texture> thisTexture = m_texture;

		if (thisTexture && !thisTexture->loadTask.is_done())
		{
			// Wait until the texture loads before updating the material on the render thread
			m_texture->loadTask.then([renderThreadData, thisResource, thisTexture]
			{
				// Update material on render thread
				Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisTexture](RenderTaskEnvironment& rte)
				{
					auto srv = thisTexture->GetSRV();
					thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
				});
			});
		}
		else
		{
			// Texture is either null or fully loaded.  Either way, we can update the material on the render thread now
			Renderer::GetInstance().EnqueueTask([renderThreadData, thisResource, thisTexture](RenderTaskEnvironment& rte)
			{
				ID3D11ShaderResourceView* srv = thisTexture ? thisTexture->GetSRV() : nullptr;
				thisResource->UpdateResourceOnRenderThread(renderThreadData.get(), srv);
			});
		}
	}
}


void MaterialResource::CreateRenderThreadData(shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceSRV<5>& resource)
{
	m_renderThreadData = materialData;

	for (uint32_t i = 0; i < 5; ++i)
	{
		const auto& binding = resource.binding[i];
		m_shaderSlots[i].first = binding.tableIndex;
		m_shaderSlots[i].second = binding.tableSlot;
	}

	SetResource(m_texture);
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