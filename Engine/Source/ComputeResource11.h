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
namespace RenderThread { struct ComputeData; }


class ComputeResource : public std::enable_shared_from_this<ComputeResource>
{
public:
	ComputeResource(const std::string& name);

	const std::string& GetName() const { return m_name; }

	void SetSRV(std::shared_ptr<Texture> texture) { SetSRVInternal(texture, false); }
	void SetSRV(std::shared_ptr<DepthBuffer> buffer, bool stencil = false) { SetSRVInternal(buffer, stencil, false); }
	void SetSRV(std::shared_ptr<ColorBuffer> buffer) { SetSRVInternal(buffer, false); }
	void SetSRV(std::shared_ptr<GpuBuffer> buffer) { SetSRVInternal(buffer, false); }

	void SetUAV(std::shared_ptr<ColorBuffer> buffer) { SetUAVInternal(buffer, false); }
	void SetUAV(std::shared_ptr<GpuBuffer> buffer) { SetUAVInternal(buffer, false); }

	void SetTextureImmediate(std::shared_ptr<Texture> texture) { SetSRVInternal(texture, true); }
	void SetSRVImmediate(std::shared_ptr<Texture> texture) { SetSRVInternal(texture, true); }
	void SetSRVImmediate(std::shared_ptr<DepthBuffer> buffer, bool stencil = false) { SetSRVInternal(buffer, stencil, true); }
	void SetSRVImmediate(std::shared_ptr<ColorBuffer> buffer) { SetSRVInternal(buffer, true); }

	void SetUAVImmediate(std::shared_ptr<ColorBuffer> buffer) { SetUAVInternal(buffer, true); }
	void SetUAVImmediate(std::shared_ptr<GpuBuffer> buffer) { SetUAVInternal(buffer, true); }

	// SRV
	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> materialData, const ShaderReflection::ResourceSRV<1>& resource);
	// UAV
	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> materialData, const ShaderReflection::ResourceUAV<1>& resource);

private:
	void SetSRVInternal(std::shared_ptr<Texture> texture, bool bImmediate);
	void SetSRVInternal(std::shared_ptr<DepthBuffer> buffer, bool stencil, bool bImmediate);
	void SetSRVInternal(std::shared_ptr<ColorBuffer> buffer, bool bImmediate);
	void SetSRVInternal(std::shared_ptr<GpuBuffer> buffer, bool bImmediate);

	void SetUAVInternal(std::shared_ptr<ColorBuffer> buffer, bool bImmediate);
	void SetUAVInternal(std::shared_ptr<GpuBuffer> buffer, bool bImmediate);

	void UpdateResourceOnRenderThread(RenderThread::ComputeData* materialData, ID3D11ShaderResourceView* srv);
	void UpdateResourceOnRenderThread(RenderThread::ComputeData* materialData, ID3D11UnorderedAccessView* uav);

	void DispatchToRenderThread(ID3D11ShaderResourceView* srv, bool bImmediate);
	void DispatchToRenderThread(ID3D11UnorderedAccessView* uav, bool bImmediate);
	void DispatchToRenderThreadNoLock(std::shared_ptr<RenderThread::ComputeData> materialData, ID3D11ShaderResourceView* srv, bool bImmediate);

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
	std::weak_ptr<RenderThread::ComputeData>	m_renderThreadData;
	uint32_t									m_bindingTable{ kInvalid };
	uint32_t									m_bindingSlot{ kInvalid };
};

} // namespace Kodiak