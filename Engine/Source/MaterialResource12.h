// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "ShaderReflection.h"

namespace Kodiak
{

// Forward declarations
class Texture;
enum class ShaderResourceType;
enum class ShaderResourceDimension;
namespace RenderThread { struct MaterialData; }


class MaterialResource : public std::enable_shared_from_this<MaterialResource>
{
public:
	MaterialResource(const std::string& name);

	const std::string& GetName() const { return m_name; }

	void SetResource(std::shared_ptr<Texture> texture);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceSRV<5>& resource);;

private:
	void UpdateResourceOnRenderThread(RenderThread::MaterialData* materialData, D3D12_CPU_DESCRIPTOR_HANDLE srv);

private:
	const std::string			m_name;
	ShaderResourceType			m_type;
	ShaderResourceDimension		m_dimension;

	std::shared_ptr<Texture>	m_texture;

	// Render thread data
	std::array<std::pair<uint32_t, uint32_t>, 5>		m_shaderSlots;
	std::weak_ptr<RenderThread::MaterialData>			m_renderThreadData;
};

} // namespace Kodiak