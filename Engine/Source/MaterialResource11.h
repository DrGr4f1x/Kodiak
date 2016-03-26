// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

// TODO: get rid of this header
#include "Effect.h"

namespace Kodiak
{

// Forward declarations
class Texture;
enum class ShaderResourceType;
enum class ShaderResourceDimension;
namespace RenderThread
{
struct MaterialData;
class MaterialResourceData;
}


class MaterialResource
{
public:
	MaterialResource(const std::string& name);

	const std::string& GetName() const { return m_name; }

	void SetResource(std::shared_ptr<Texture> texture);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const Effect::ResourceSRV& resource);

private:
	const std::string m_name;
	ShaderResourceType		m_type;
	ShaderResourceDimension m_dimension;

	std::shared_ptr<Texture>	m_texture;

	std::shared_ptr<RenderThread::MaterialResourceData>	m_renderThreadData;
};


namespace RenderThread
{

class MaterialResourceData
{
	friend class MaterialResource;
public:
	MaterialResourceData(std::shared_ptr<MaterialData> materialData);

	void SetResource(ID3D11ShaderResourceView* srv);
	
private:
	std::array<std::pair<uint32_t, uint32_t>, 5>		m_shaderSlots;
	std::weak_ptr<MaterialData>							m_materialData;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_srv;
};

} // namespace RenderThread

} // namespace Kodiak