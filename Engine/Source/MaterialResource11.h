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
class GpuBuffer;
class Texture;
enum class ShaderResourceType;
enum class ShaderResourceDimension;
namespace RenderThread { struct MaterialData; }


class MaterialResource : public std::enable_shared_from_this<MaterialResource>
{
public:
	MaterialResource(const std::string& name);

	const std::string& GetName() const { return m_name; }

	void SetSRV(std::shared_ptr<Texture> texture) { SetSRVInternal(texture, false); }
	void SetSRV(std::shared_ptr<DepthBuffer> buffer, bool stencil = false) { SetSRVInternal(buffer, stencil, false); }
	void SetSRV(std::shared_ptr<ColorBuffer> buffer) { SetSRVInternal(buffer, false); }
	void SetSRV(std::shared_ptr<GpuBuffer> buffer) { SetSRVInternal(buffer, false); }

	void SetUAV(std::shared_ptr<ColorBuffer> buffer) { SetUAVInternal(buffer, false); }
	void SetUAV(std::shared_ptr<GpuBuffer> buffer) { SetUAVInternal(buffer, false); }

	void SetSRVImmediate(std::shared_ptr<Texture> texture) { SetSRVInternal(texture, true); }
	void SetSRVImmediate(std::shared_ptr<DepthBuffer> buffer, bool stencil = false) { SetSRVInternal(buffer, stencil, true); }
	void SetSRVImmediate(std::shared_ptr<ColorBuffer> buffer) { SetSRVInternal(buffer, true); }
	void SetSRVImmediate(std::shared_ptr<GpuBuffer> buffer) { SetSRVInternal(buffer, true); }

	void SetUAVImmediate(std::shared_ptr<ColorBuffer> buffer) { SetUAVInternal(buffer, true); }
	void SetUAVImmediate(std::shared_ptr<GpuBuffer> buffer) { SetUAVInternal(buffer, true); }

	void CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceSRV<5>& resource);
	void CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceUAV<5>& resource);

private:
	void SetSRVInternal(std::shared_ptr<Texture> texture, bool bImmediate);
	void SetSRVInternal(std::shared_ptr<DepthBuffer> buffer, bool stencil, bool bImmediate);
	void SetSRVInternal(std::shared_ptr<ColorBuffer> buffer, bool bImmediate);
	void SetSRVInternal(std::shared_ptr<GpuBuffer> buffer, bool bImmediate);

	void SetUAVInternal(std::shared_ptr<ColorBuffer> buffer, bool bImmediate);
	void SetUAVInternal(std::shared_ptr<GpuBuffer> buffer, bool bImmediate);

	void UpdateResourceOnRenderThread(RenderThread::MaterialData* materialData, ID3D11ShaderResourceView* srv);
	void UpdateResourceOnRenderThread(RenderThread::MaterialData* materialData, ID3D11UnorderedAccessView* uav);
	void DispatchToRenderThread(ID3D11ShaderResourceView* srv, bool immediate);
	void DispatchToRenderThread(ID3D11UnorderedAccessView* uav, bool immediate);
	void DispatchToRenderThreadNoLock(std::shared_ptr<RenderThread::MaterialData> materialData, ID3D11ShaderResourceView* srv, bool immediate);

	inline void SetCachedResources(std::shared_ptr<Texture> texture, std::shared_ptr<ColorBuffer> colorBuffer,
		std::shared_ptr<DepthBuffer> depthBuffer, std::shared_ptr<GpuBuffer> gpuBuffer, bool stencil)
	{
		m_texture = texture;
		m_colorBuffer = colorBuffer;
		m_depthBuffer = depthBuffer;
		m_gpuBuffer = gpuBuffer;
		m_stencil = stencil;
	}

private:
	const std::string				m_name;
	ShaderResourceType				m_type;
	ShaderResourceDimension			m_dimension;

	std::shared_ptr<Texture>		m_texture;
	std::shared_ptr<ColorBuffer>	m_colorBuffer;
	std::shared_ptr<DepthBuffer>	m_depthBuffer;
	std::shared_ptr<GpuBuffer>		m_gpuBuffer;
	bool							m_stencil{ false };

	// Render thread data
	std::array<std::pair<uint32_t, uint32_t>, 5>		m_shaderSlots;
	std::weak_ptr<RenderThread::MaterialData>			m_renderThreadData;
};

} // namespace Kodiak