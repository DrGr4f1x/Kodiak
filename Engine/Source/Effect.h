// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "Format.h"
#include "PipelineState.h"
#include "Shader.h"

#include <array>
#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
enum class PrimitiveTopologyType;


class BaseEffect
{
public:
	concurrency::task<void> loadTask;

public:
	// Constructor/destructor
	BaseEffect();
	virtual ~BaseEffect() {}

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

	void SetVertexShaderPath(const std::string& path, const std::string& file)
	{
		SetVertexShaderPath(ShaderPath(path, file));
	}
	void SetVertexShaderPath(const ShaderPath& shaderPath);

	void SetDomainShaderPath(const std::string& path, const std::string& file)
	{
		SetDomainShaderPath(ShaderPath(path, file));
	}
	void SetDomainShaderPath(const ShaderPath& shaderPath);

	void SetHullShaderPath(const std::string& path, const std::string& file)
	{
		SetHullShaderPath(ShaderPath(path, file));
	}
	void SetHullShaderPath(const ShaderPath& shaderPath);

	void SetGeometryShaderPath(const std::string& path, const std::string& file)
	{
		SetGeometryShaderPath(ShaderPath(path, file));
	}
	void SetGeometryShaderPath(const ShaderPath& shaderPath);

	void SetPixelShaderPath(const std::string& path, const std::string& file)
	{
		SetPixelShaderPath(ShaderPath(path, file));
	}
	void SetPixelShaderPath(const ShaderPath& shaderPath);

	void SetBlendState(const BlendStateDesc& desc) { m_blendStateDesc = desc; }
	void SetDepthStencilState(const DepthStencilStateDesc& desc) { m_depthStencilStateDesc = desc; }
	void SetRasterizerState(const RasterizerStateDesc& desc) { m_rasterizerStateDesc = desc; }

	void SetSampleMask(uint32_t mask) { m_sampleMask = mask; }
	void SetPrimitiveTopology(PrimitiveTopologyType topology);
	void SetRenderTargetFormat(ColorFormat colorFormat, DepthFormat depthFormat, uint32_t msaaCount = 1, uint32_t msaaQuality = 0)
	{
		SetRenderTargetFormats(1, &colorFormat, depthFormat, msaaCount, msaaQuality);
	}
	void SetRenderTargetFormats(uint32_t numRTVs, const ColorFormat* colorFormats, DepthFormat depthFormat, uint32_t msaaCount = 1,
		uint32_t msaaQuality = 0);

	// Finalize, doing any additional work after data fields are assigned
	virtual void Finalize() {}

protected:
	std::string							m_name;

	ShaderPath							m_shaderPaths[5];

	std::shared_ptr<VertexShader>		m_vertexShader;
	std::shared_ptr<DomainShader>		m_domainShader;
	std::shared_ptr<HullShader>			m_hullShader;
	std::shared_ptr<GeometryShader>		m_geometryShader;
	std::shared_ptr<PixelShader>		m_pixelShader;

	BlendStateDesc						m_blendStateDesc;
	DepthStencilStateDesc				m_depthStencilStateDesc;
	RasterizerStateDesc					m_rasterizerStateDesc;

	// Miscellaneous fields (mostly for DX12)
	uint32_t							m_sampleMask{ 0xFFFFFFFFu };
	PrimitiveTopologyType				m_topology;
	std::array<ColorFormat, 8>			m_colorFormats;
	DepthFormat							m_depthFormat;
	uint32_t							m_numRenderTargets{ 0 };
	uint32_t							m_msaaCount{ 1 };
	uint32_t							m_msaaQuality{ 0 };

	// Shader bindings
	std::vector<ShaderBindingDesc>		m_shaderBindings;
};

} // namespace Kodiak


#if defined(DX12)
#include "Effect12.h"
#elif defined(DX11)
#include "Effect11.h"
#elif defined(VK)
#include "EffectVk.h"
#else
#error No graphics API defined!
#endif