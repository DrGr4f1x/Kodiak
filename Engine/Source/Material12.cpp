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


// Utility methods for processing material parameter bindings
namespace
{

const ShaderConstantBufferDesc* GetPerMaterialConstantBufferDesc(const BaseShader* shader)
{
	const auto cbufferSlot = GetPerMaterialConstantsSlot();
	const auto& cbufferName = GetPerMaterialConstantsName();

	const auto& cbuffers = shader->GetConstantBuffers();
	const auto numCBuffers = cbuffers.size();

	for (size_t i = 0; i < numCBuffers; ++i)
	{
		const auto& cbuffer = cbuffers[i];
		if (cbuffer.name == cbufferName)
		{
			assert(cbuffer.registerSlot == cbufferSlot);
			return &cbuffers[i];
		}
	}

	return nullptr;
}


void ValidateShaderConstants(const BaseShader* shader, const string& cbufferName, uint32_t cbufferSlot, bool& bufferFound, uint32_t& size)
{
	const auto& cbuffers = shader->GetConstantBuffers();
	
	for (auto it = begin(cbuffers), last = end(cbuffers); it != last; ++it)
	{
		const auto& cbuffer = *it;
		if (cbuffer.name == cbufferName)
		{
			assert(cbuffer.registerSlot == cbufferSlot);
			if (bufferFound)
			{
				assert(cbuffer.size == size);
			}
			else
			{
				bufferFound = true;
				size = cbuffer.size;
			}
			break;
		}
	}
}

} // anonymous namespace


void Material::ConfigureRootSignature(const ShaderState& shaderState)
{
	// Verify we have a vertex shader (mandatory)
	assert(shaderState.vertexShader);

	// Verify that all shader stages using PerViewConstants have the same size buffer in the correct register slot
	bool usesPerViewConstants = ValidateConstantBuffers(shaderState, GetPerViewConstantsName(), GetPerViewConstantsSlot());

	// Verify that all shader stages using PerObjectConstants have the same size buffer in the correct register slot
	bool usesPerObjectConstants = ValidateConstantBuffers(shaderState, GetPerObjectConstantsName(), GetPerObjectConstantsSlot());

	uint32_t totalConstantBuffers = 0;
	totalConstantBuffers += usesPerViewConstants ? 1 : 0;
	totalConstantBuffers += usesPerObjectConstants ? 1 : 0;

	ConfigureRootConstantBuffers(shaderState, totalConstantBuffers);
	

	// TODO: Static samplers
	uint32_t totalStaticSamplers = 0;
}


void Material::ConfigureRootConstantBuffers(const ShaderState& shaderState, uint32_t& totalConstantBuffers)
{
	// Create records for material constant buffers
	m_constantBuffers.clear();
	m_constantBuffers.reserve(5);
	m_constantBuffers.emplace_back(D3D12_SHADER_VISIBILITY_VERTEX);
	m_constantBuffers.emplace_back(D3D12_SHADER_VISIBILITY_DOMAIN);
	m_constantBuffers.emplace_back(D3D12_SHADER_VISIBILITY_HULL);
	m_constantBuffers.emplace_back(D3D12_SHADER_VISIBILITY_GEOMETRY);
	m_constantBuffers.emplace_back(D3D12_SHADER_VISIBILITY_PIXEL);

	// Vertex shader's per-material constants
	auto cbufferDesc = GetPerMaterialConstantBufferDesc(shaderState.vertexShader.get());
	if (cbufferDesc)
	{
		m_constantBuffers[0].buffer = make_unique<ConstantBuffer>();
		m_constantBuffers[0].buffer->Create(cbufferDesc->size, Usage::Dynamic);

		m_constantBuffers[0].bufferDesc = cbufferDesc;
		m_constantBuffers[0].rootSignatureSlot = totalConstantBuffers++;
	}

	// Domain shader's per-material constants
	cbufferDesc = GetPerMaterialConstantBufferDesc(shaderState.domainShader.get());
	if (cbufferDesc)
	{
		m_constantBuffers[1].buffer = make_unique<ConstantBuffer>();
		m_constantBuffers[1].buffer->Create(cbufferDesc->size, Usage::Dynamic);

		m_constantBuffers[1].bufferDesc = cbufferDesc;
		m_constantBuffers[1].rootSignatureSlot = totalConstantBuffers++;
	}

	// Hull shader's per-material constants
	cbufferDesc = GetPerMaterialConstantBufferDesc(shaderState.hullShader.get());
	if (cbufferDesc)
	{
		m_constantBuffers[2].buffer = make_unique<ConstantBuffer>();
		m_constantBuffers[2].buffer->Create(cbufferDesc->size, Usage::Dynamic);

		m_constantBuffers[2].bufferDesc = cbufferDesc;
		m_constantBuffers[2].rootSignatureSlot = totalConstantBuffers++;
	}

	// Geometry shader's per-material constants
	cbufferDesc = GetPerMaterialConstantBufferDesc(shaderState.geometryShader.get());
	if (cbufferDesc)
	{
		m_constantBuffers[3].buffer = make_unique<ConstantBuffer>();
		m_constantBuffers[3].buffer->Create(cbufferDesc->size, Usage::Dynamic);

		m_constantBuffers[3].bufferDesc = cbufferDesc;
		m_constantBuffers[3].rootSignatureSlot = totalConstantBuffers++;
	}

	// Domain shader's per-material constants
	cbufferDesc = GetPerMaterialConstantBufferDesc(shaderState.pixelShader.get());
	if (cbufferDesc)
	{
		m_constantBuffers[4].buffer = make_unique<ConstantBuffer>();
		m_constantBuffers[4].buffer->Create(cbufferDesc->size, Usage::Dynamic);

		m_constantBuffers[4].bufferDesc = cbufferDesc;
		m_constantBuffers[4].rootSignatureSlot = totalConstantBuffers++;
	}
}


bool Material::ValidateConstantBuffers(const ShaderState& shaderState, const string& cbufferName, uint32_t cbufferSlot)
{
	bool bufferFound = false;
	uint32_t size = 0;

	// Get data from mandatory vertex shader
	ValidateShaderConstants(shaderState.vertexShader.get(), cbufferName, cbufferSlot, bufferFound, size);

	// Optional domain shader
	if (shaderState.domainShader)
	{
		ValidateShaderConstants(shaderState.domainShader.get(), cbufferName, cbufferSlot, bufferFound, size);
	}

	// Optional hull shader
	if (shaderState.hullShader)
	{
		ValidateShaderConstants(shaderState.hullShader.get(), cbufferName, cbufferSlot, bufferFound, size);
	}

	// Optional geometry shader
	if(shaderState.geometryShader)
	{
		ValidateShaderConstants(shaderState.geometryShader.get(), cbufferName, cbufferSlot, bufferFound, size);
	}

	// Optional pixel shader
	if (shaderState.pixelShader)
	{
		ValidateShaderConstants(shaderState.pixelShader.get(), cbufferName, cbufferSlot, bufferFound, size);
	}

	return bufferFound;
}