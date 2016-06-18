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
	BaseEffect(const std::string& name);
	virtual ~BaseEffect() {}

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

	void SetVertexShaderPath(const std::string& shaderPath);
	void SetDomainShaderPath(const std::string& shaderPath);
	void SetHullShaderPath(const std::string& shaderPath);
	void SetGeometryShaderPath(const std::string& path);
	void SetPixelShaderPath(const std::string& path);

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

	std::string							m_shaderPaths[5];

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