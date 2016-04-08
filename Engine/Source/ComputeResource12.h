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
class DepthBuffer;
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
	void SetSRV(std::shared_ptr<Texture> texture) { SetTexture(texture); }
	void SetSRV(std::shared_ptr<DepthBuffer> buffer, bool stencil = false);
	void SetSRV(std::shared_ptr<ColorBuffer> buffer);
	void SetUAV(std::shared_ptr<ColorBuffer> buffer);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::ResourceSRV<1>& resource,
		uint32_t destCpuHandleSlot);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::ResourceUAV<1>& resource,
		uint32_t destCpuHandleSlot);

private:
	void UpdateResourceOnRenderThread(RenderThread::ComputeData* materialData, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);

private:
	const std::string				m_name;
	ShaderResourceType				m_type;
	ShaderResourceDimension			m_dimension;

	std::shared_ptr<Texture>		m_texture;
	std::shared_ptr<ColorBuffer>	m_colorBuffer;
	std::shared_ptr<DepthBuffer>	m_depthBuffer;
	bool							m_stencil{ false };

	// Render thread data
	std::weak_ptr<RenderThread::ComputeData>	m_renderThreadData;
	uint32_t									m_binding{ kInvalid };
};

} // namespace Kodiak