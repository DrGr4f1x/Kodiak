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

	void CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceSRV<5>& resource);
	void CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::ResourceUAV<5>& resource);

private:
	void SetSRVInternal(Texture& texture, bool bImmediate);
	void SetSRVInternal(DepthBuffer& buffer, bool stencil, bool bImmediate);
	void SetSRVInternal(ColorBuffer& buffer, bool bImmediate);
	void SetSRVInternal(GpuBuffer& buffer, bool bImmediate);

	void SetUAVInternal(ColorBuffer& buffer, bool bImmediate);
	void SetUAVInternal(GpuBuffer& buffer, bool bImmediate);

	void UpdateResourceOnRenderThread(RenderThread::MaterialData* materialData, ID3D11ShaderResourceView* srv);
	void UpdateResourceOnRenderThread(RenderThread::MaterialData* materialData, ID3D11UnorderedAccessView* uav);
	void DispatchToRenderThread(ID3D11ShaderResourceView* srv, bool immediate);
	void DispatchToRenderThread(ID3D11UnorderedAccessView* uav, bool immediate);
	
	inline void SetCachedResources(ID3D11ShaderResourceView* srv, ID3D11UnorderedAccessView* uav)
	{
		m_srv = srv;
		m_uav = uav;
	}

private:
	const std::string				m_name;
	ShaderResourceType				m_type;
	ShaderResourceDimension			m_dimension;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_srv;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	m_uav;

	// Render thread data
	std::array<std::pair<uint32_t, uint32_t>, 5>		m_shaderSlots;
	std::weak_ptr<RenderThread::MaterialData>			m_renderThreadData;
};

} // namespace Kodiak