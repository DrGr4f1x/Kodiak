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

	void SetSRV(Texture& texture) { SetSRVInternal(texture, false); }
	void SetSRV(DepthBuffer& buffer, bool stencil = false) { SetSRVInternal(buffer, stencil, false); }
	void SetSRV(ColorBuffer& buffer) { SetSRVInternal(buffer, false); }
	void SetSRV(GpuBuffer& buffer) { SetSRVInternal(buffer, false); }

	void SetUAV(ColorBuffer& buffer) { SetUAVInternal(buffer, false); }
	void SetUAV(GpuBuffer& buffer) { SetUAVInternal(buffer, false); }

	void SetSRVImmediate(Texture& texture) { SetSRVInternal(texture, true); }
	void SetSRVImmediate(DepthBuffer& buffer, bool stencil = false) { SetSRVInternal(buffer, stencil, true); }
	void SetSRVImmediate(ColorBuffer& buffer) { SetSRVInternal(buffer, true); }
	void SetSRVImmediate(GpuBuffer& buffer) { SetSRVInternal(buffer, true); }

	void SetUAVImmediate(ColorBuffer& buffer) { SetUAVInternal(buffer, true); }
	void SetUAVImmediate(GpuBuffer& buffer) { SetUAVInternal(buffer, true); }

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::ResourceSRV<1>& resource,
		uint32_t destCpuHandleSlot);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::ResourceUAV<1>& resource,
		uint32_t destCpuHandleSlot);

private:
	void SetSRVInternal(Texture& texture, bool immediate);
	void SetSRVInternal(DepthBuffer& buffer, bool stencil, bool immediate);
	void SetSRVInternal(ColorBuffer& buffer, bool immediate);
	void SetSRVInternal(GpuBuffer& buffer, bool immediate);
	
	void SetUAVInternal(ColorBuffer& buffer, bool immediate);
	void SetUAVInternal(GpuBuffer& buffer, bool immediate);

	void UpdateResourceOnRenderThread(RenderThread::ComputeData* materialData, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);
	void DispatchToRenderThread(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, bool bImmediate);
	
private:
	const std::string				m_name;
	ShaderResourceType				m_type;
	ShaderResourceDimension			m_dimension;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_cpuHandle;

	// Render thread data
	std::weak_ptr<RenderThread::ComputeData>	m_renderThreadData;
	uint32_t									m_binding{ kInvalid };
};

} // namespace Kodiak