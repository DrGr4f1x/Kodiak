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
class ColorBuffer;
class Texture;
enum class ShaderResourceType;
enum class ShaderResourceDimension;
namespace RenderThread { struct ComputeData; }


class ComputeResource : public std::enable_shared_from_this<ComputeResource>
{
public:
	ComputeResource(const std::string& name);

	const std::string& GetName() const { return m_name; }

	void SetTexture(std::shared_ptr<Texture> texture);
	void SetUAV(std::shared_ptr<ColorBuffer> buffer);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> materialData, const ShaderReflection::ResourceSRV<1>& resource);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> materialData, const ShaderReflection::ResourceUAV<1>& resource);

private:
	void UpdateResourceOnRenderThread(RenderThread::ComputeData* materialData, ID3D11ShaderResourceView* srv);
	void UpdateResourceOnRenderThread(RenderThread::ComputeData* materialData, ID3D11UnorderedAccessView* uav);

private:
	const std::string				m_name;
	ShaderResourceType				m_type;
	ShaderResourceDimension			m_dimension;

	std::shared_ptr<Texture>		m_texture;  // For SRV resource
	std::shared_ptr<ColorBuffer>	m_buffer;	// For UAV resource

												// Render thread data
	std::weak_ptr<RenderThread::ComputeData>	m_renderThreadData;
	uint32_t									m_bindingTable{ kInvalid };
	uint32_t									m_bindingSlot{ kInvalid };
};

} // namespace Kodiak