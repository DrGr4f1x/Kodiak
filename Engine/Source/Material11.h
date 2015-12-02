// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class DomainShader;
class GraphicsPSO;
class HullShader;
class VertexShader;
struct BlendStateDesc;
struct RasterizerStateDesc;
struct DepthStencilStateDesc;


class Material
{
public:
	Material();
	Material(const std::string& name);

	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	void SetVertexShaderPath(const std::string& shaderPath, const std::string& shaderFile);
	void SetVertexShader(std::shared_ptr<VertexShader> vertexShader);

	void SetDomainShaderPath(const std::string& shaderPath, const std::string& shaderFile);
	void SetDomainShader(std::shared_ptr<DomainShader> domainShader);

	void SetHullShaderPath(const std::string& shaderPath, const std::string& shaderFile);
	void SetHullShader(std::shared_ptr<HullShader> hullShader);

public:
	concurrency::task<void> loadTask;

private:
	// Graphics objects
	std::shared_ptr<GraphicsPSO>	m_pso;
	std::shared_ptr<DomainShader>	m_domainShader;
	std::shared_ptr<HullShader>		m_hullShader;
	std::shared_ptr<VertexShader>	m_vertexShader;


	// Material properties
	std::string						m_name;
	bool							m_isTaskValid{ false };
	bool							m_isFinalized{ false };
};

} // namespace Kodiak