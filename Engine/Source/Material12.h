// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "PipelineState12.h"
#include "Shader.h"

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class RootSignature;


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
	std::shared_ptr<RootSignature>	m_rootSig;
	std::shared_ptr<GraphicsPSO>	m_pso;

	// Material properties
	std::string						m_name;
	bool							m_isReady{ false };
};


} // namespace Kodiak