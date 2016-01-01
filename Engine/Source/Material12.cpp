// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Material.h"

#include "RootSignature12.h"


using namespace Kodiak;
using namespace std;


static const uint32_t sPerViewConstantsSlot = 0;
static const uint32_t sPerObjectConstantsSlot = 1;
static const uint32_t sPerMaterialConstantsSlot = 2;


namespace Kodiak
{

size_t ComputeHash(const MaterialDesc& desc)
{
	return ComputeBaseHash(desc);
}

} // namespace Kodiak


Material::Material(const string& name)
	: m_name(name)
{}


void Material::BindParameters(const ShaderState& shaderState)
{
	ConfigureRootSignature(shaderState);
}


void Material::SetupPSO(const MaterialDesc& desc)
{
	m_pso->SetBlendState(desc.blendStateDesc);
	m_pso->SetDepthStencilState(desc.depthStencilStateDesc);
	m_pso->SetRasterizerState(desc.rasterizerStateDesc);
}


void Material::ConfigureRootSignature(const ShaderState& shaderState)
{
	// Validate per-view constant buffer bindings
	auto valid = ValidateConstants(shaderState, 0);
	assert(valid);
}


bool Material::ValidateConstants(const ShaderState& shaderState, uint32_t slot)
{
	bool valid = true;
	bool bufferFound = false;
	uint32_t size = 0;

	// Get data from mandatory vertex shader
	{
		assert(shaderState.vertexShader);
		const auto& cbuffers = shaderState.vertexShader->GetConstantBuffers();
		if (!cbuffers.empty())
		{

		}
	}

	return valid;
}