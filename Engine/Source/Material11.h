// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "PipelineState11.h"

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class DomainShader;
class GeometryShader;
class HullShader;
class PixelShader;
class VertexShader;


struct MaterialDesc
{
	std::string name;

	std::string vertexShaderPath;
	std::string vertexShaderFile;

	std::string domainShaderPath;
	std::string domainShaderFile;

	std::string hullShaderPath;
	std::string hullShaderFile;

	std::string geometryShaderPath;
	std::string geometryShaderFile;

	std::string pixelShaderPath;
	std::string pixelShaderFile;

	BlendStateDesc			blendStateDesc;
	DepthStencilStateDesc	depthStencilStateDesc;
	RasterizerStateDesc		rasterizerStateDesc;
};


size_t ComputeHash(const MaterialDesc& desc);


class Material
{
	friend class MaterialManager;

public:
	explicit Material(const MaterialDesc& desc);
	
	const std::string& GetName() const { return m_name; }
	
public:
	concurrency::task<void> loadTask;

private:
	void Create(const MaterialDesc& desc);

private:
	// Graphics objects
	std::shared_ptr<GraphicsPSO>	m_pso;
	std::shared_ptr<VertexShader>	m_vertexShader;
	std::shared_ptr<DomainShader>	m_domainShader;
	std::shared_ptr<HullShader>		m_hullShader;
	std::shared_ptr<GeometryShader>	m_geometryShader;
	std::shared_ptr<PixelShader>	m_pixelShader;


	// Material properties
	std::string						m_name;
	bool							m_isTaskValid{ false };
};

} // namespace Kodiak