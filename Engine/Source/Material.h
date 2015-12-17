// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "PipelineState.h"
#include "Shader.h"

namespace Kodiak
{

struct MaterialDesc
{
	std::string		name;

	ShaderPath		vertexShaderPath;
	ShaderPath		domainShaderPath;
	ShaderPath		hullShaderPath;
	ShaderPath		geometryShaderPath;
	ShaderPath		pixelShaderPath;

	BlendStateDesc			blendStateDesc;
	DepthStencilStateDesc	depthStencilStateDesc;
	RasterizerStateDesc		rasterizerStateDesc;
};


std::shared_ptr<MaterialDesc> CreateMaterialDesc();


size_t ComputeBaseHash(const MaterialDesc& desc);
size_t ComputeHash(const MaterialDesc& desc);


struct ShaderState
{
	std::shared_ptr<VertexShader>		vertexShader;
	std::shared_ptr<DomainShader>		domainShader;
	std::shared_ptr<HullShader>			hullShader;
	std::shared_ptr<GeometryShader>		geometryShader;
	std::shared_ptr<PixelShader>		pixelShader;
};


class Material
{
	friend class MaterialManager;

public:
	const std::string& GetName() const { return m_name; }

private:
	// To be called by MaterialManager (any thread)
	Material(const std::string& name);
	void BindParameters(const ShaderState& shaderState);
	void SetupPSO(const MaterialDesc& desc);

private:
	// Graphics objects
	std::shared_ptr<GraphicsPSO>	m_pso;

	// Material properties
	std::string						m_name;
	bool							m_isReady{ false };
};


} // namespace Kodiak