// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#if 0

#include "Effect.h"

#include "InputLayout12.h"
#include "Material.h"
#include "RenderEnums.h"
#include "RootSignature12.h"
#include "Shader.h"

using namespace Kodiak;
using namespace std;


namespace
{
	const D3D12_SHADER_VISIBILITY s_shaderVisibility[] =
	{
		D3D12_SHADER_VISIBILITY_VERTEX,
		D3D12_SHADER_VISIBILITY_DOMAIN,
		D3D12_SHADER_VISIBILITY_HULL,
		D3D12_SHADER_VISIBILITY_GEOMETRY,
		D3D12_SHADER_VISIBILITY_PIXEL
	};

	const Kodiak::ShaderType s_shaderStage[] =
	{
		ShaderType::Vertex,
		ShaderType::Domain,
		ShaderType::Hull,
		ShaderType::Geometry,
		ShaderType::Pixel
	};

} // anonymous namespace


void Effect::Finalize()
{
	loadTask = loadTask.then([this]
	{
		BuildEffectSignature();
		BuildRootSignature();
		BuildPSO();
	});
}


void Effect::BuildEffectSignature()
{
	// Clear out old effect signature data (shouldn't have any, but just to be safe...)
	m_signature.perViewDataSize = 0;
	m_signature.perObjectDataSize = 0;
	fill(begin(m_signature.perShaderDescriptorCount), end(m_signature.perShaderDescriptorCount), 0);
	m_signature.numRootParameters = 0;
	m_signature.numStaticSamplers = 0;

	// Determine how many descriptors each shader has
	ProcessShaderBindings(0, m_vertexShader.get());
	ProcessShaderBindings(1, m_domainShader.get());
	ProcessShaderBindings(2, m_hullShader.get());
	ProcessShaderBindings(3, m_geometryShader.get());
	ProcessShaderBindings(4, m_pixelShader.get());

	// Determine how many root parameters we need
	m_signature.numRootParameters += (m_signature.perViewDataSize > 0) ? 1 : 0;
	m_signature.numRootParameters += (m_signature.perObjectDataSize > 0) ? 1 : 0;
	for (uint32_t i = 0; i < 5; ++i)
	{
		m_signature.numRootParameters += (m_signature.perShaderDescriptorCount[i] > 0) ? 1 : 0;
	}

	// TODO: static samplers
}


void Effect::BuildRootSignature()
{
	m_rootSig = make_shared<RootSignature>(m_signature.numRootParameters, m_signature.numStaticSamplers);
	auto& rootSig = *m_rootSig;

	uint32_t currentParameter = 0;

	// Build a list of the shader descriptors, so we can pull cbuffer and SRV binding information
	vector<ShaderBindingDesc> shaderBindings(5);
	static const ShaderBindingDesc dummyDesc;
	shaderBindings.emplace_back(m_vertexShader   ? m_vertexShader->GetBindingSignature()   : dummyDesc);
	shaderBindings.emplace_back(m_domainShader   ? m_domainShader->GetBindingSignature()   : dummyDesc);
	shaderBindings.emplace_back(m_hullShader     ? m_hullShader->GetBindingSignature()     : dummyDesc);
	shaderBindings.emplace_back(m_geometryShader ? m_geometryShader->GetBindingSignature() : dummyDesc);
	shaderBindings.emplace_back(m_pixelShader    ? m_pixelShader->GetBindingSignature()    : dummyDesc);


	// Reserve a slot for the per-view data
	if (m_signature.perViewDataSize > 0)
	{
		rootSig[currentParameter++].InitAsConstantBuffer(GetPerViewConstantsSlot());
	}

	// Reserve a slot for the per-object data
	if (m_signature.perObjectDataSize > 0)
	{
		rootSig[currentParameter++].InitAsConstantBuffer(GetPerObjectConstantsSlot());
	}

	// Reserve slots for the material data and SRVs used by the shaders
	for (uint32_t i = 0; i < 5; ++i)
	{
		if (m_signature.perShaderDescriptorCount[i] > 0)
		{
			// If we have more than one parameter, use a descriptor table
			if (m_signature.perShaderDescriptorCount[i] > 1)
			{
				const auto numCBuffers = shaderBindings[i].cbuffers.size();
				const auto numResources = shaderBindings[i].resources.size();

				const auto numTableSlots = numCBuffers + numResources;
				rootSig[currentParameter].InitAsDescriptorTable(static_cast<uint32_t>(numTableSlots), s_shaderVisibility[i]);

				// Setup cbuffers in the descriptor table, and build cbuffer descriptions for the effect
				for (size_t j = 0; j < numCBuffers; ++j)
				{
					const auto& desc = shaderBindings[i].cbuffers[j];
					
					rootSig[currentParameter].SetTableRange(j, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, desc.registerSlot, 0);

					BuildConstantBufferDesc(desc, currentParameter, j, s_shaderStage[i]);
				}

				// Setup resources in the descriptor table, and build resource descriptions for the effect
				for (size_t j = 0; j < numResources; ++j)
				{
					const auto& desc = shaderBindings[i].resources[j];

					if (desc.type == ShaderResourceType::Texture || desc.type == ShaderResourceType::TBuffer)
					{
						rootSig[currentParameter].SetTableRange(j, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, desc.registerSlot, 0);
					}
					else
					{
						rootSig[currentParameter].SetTableRange(j, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, desc.registerSlot, 0);
					}

					BuildResourceDesc(desc, currentParameter, j, i);
				}
			}
			else
			{
				// Constant buffer
				if (shaderBindings[i].cbuffers.size() > 0)
				{
					const auto& desc = shaderBindings[i].cbuffers[0];
					rootSig[currentParameter].InitAsConstantBuffer(desc.registerSlot, s_shaderVisibility[i]);

					BuildConstantBufferDesc(desc, currentParameter, 0xFFFFFFFF, s_shaderStage[i]);
				}
				// Resource (SRV/UAV)
				else
				{
					assert(shaderBindings[i].resources.size() == 1);

					const auto& desc = shaderBindings[i].resources[0];
					
					// TBuffers and textures
					if (desc.type == ShaderResourceType::TBuffer || desc.type == ShaderResourceType::Texture)
					{
						rootSig[currentParameter].InitAsBufferSRV(desc.registerSlot, s_shaderVisibility[i]);
					}
					// UAVs
					else
					{
						rootSig[currentParameter].InitAsBufferUAV(desc.registerSlot, s_shaderVisibility[i]);
					}

					BuildResourceDesc(desc, currentParameter, 0xFFFFFFFF, i);
				}
			}
			++currentParameter;
		}
	}

	rootSig.Finalize();
}


void Effect::BuildPSO()
{
	m_pso = make_shared<GraphicsPSO>();

	m_pso->SetRootSignature(*m_rootSig);

	m_pso->SetBlendState(m_blendStateDesc);
	m_pso->SetRasterizerState(m_rasterizerStateDesc);
	m_pso->SetDepthStencilState(m_depthStencilStateDesc);

	m_pso->SetSampleMask(m_sampleMask);
	m_pso->SetPrimitiveTopology(m_topology);

	m_pso->SetRenderTargetFormats(m_numRenderTargets, &m_colorFormats[0], m_depthFormat, m_msaaCount, m_msaaQuality);
	
	m_pso->SetInputLayout(*m_vertexShader->GetInputLayout());

	m_pso->SetVertexShader(m_vertexShader.get());
	m_pso->SetDomainShader(m_domainShader.get());
	m_pso->SetHullShader(m_hullShader.get());
	m_pso->SetGeometryShader(m_geometryShader.get());
	m_pso->SetPixelShader(m_pixelShader.get());

	m_pso->Finalize();
}


void Effect::BuildConstantBufferDesc(const ShaderConstantBufferDesc& desc, uint32_t rootParameterIndex, uint32_t rootTableOffset,
	ShaderType shaderType)
{
	EffectConstantBufferDesc cbufferDesc;
	cbufferDesc.name = desc.name;
	cbufferDesc.shaderStage = shaderType;
	cbufferDesc.rootParameterIndex = rootParameterIndex;
	cbufferDesc.rootTableOffset = rootTableOffset;
	cbufferDesc.shaderRegister = desc.registerSlot;
	cbufferDesc.size = desc.size;
	m_signature.cbuffers.emplace_back(cbufferDesc);

	// TODO: Parameter bindings
}


void Effect::BuildResourceDesc(const ShaderResourceDesc& desc, uint32_t rootParameterIndex, uint32_t rootTableOffset, uint32_t shaderIndex)
{
	auto& resource = m_signature.resources[desc.name];
	assert(resource.type == ShaderResourceType::Unsupported || resource.type == desc.type);
	resource.type = desc.type;
	resource.bindings[shaderIndex].rootParameterIndex = rootParameterIndex;
	resource.bindings[shaderIndex].rootTableOffset = rootTableOffset;
}


void Effect::ProcessShaderBindings(uint32_t index, Shader* shader)
{
	if (shader)
	{
		// Validate per-view data size - must be zero or the same as the other shaders in the effect
		auto perViewDataSize = shader->GetPerViewDataSize();
		if (perViewDataSize > 0)
		{
			assert((m_signature.perViewDataSize == 0) || (m_signature.perViewDataSize == perViewDataSize));
			m_signature.perViewDataSize = perViewDataSize;
		}

		// Validate per-object data size - must be zero or the same as the other shaders in the effect
		auto perObjectDataSize = shader->GetPerObjectDataSize();
		if (perObjectDataSize > 0)
		{
			assert((m_signature.perObjectDataSize == 0) || (m_signature.perObjectDataSize == perObjectDataSize));
			m_signature.perObjectDataSize = perObjectDataSize;
		}

		const auto& shaderBinding = shader->GetBindingSignature();
		auto descriptorCount = shaderBinding.resources.size();

		const auto numCBuffers = shaderBinding.cbuffers.size();
		for (size_t i = 0; i < numCBuffers; ++i)
		{
			const auto& cbufferDesc = shaderBinding.cbuffers[i];

			// Ignore the per-view and per-object cbuffers.  Handle everything else.
			if (cbufferDesc.name != GetPerViewConstantsName() && cbufferDesc.name != GetPerObjectConstantsName())
			{
				descriptorCount++;
			}
		}

		m_signature.perShaderDescriptorCount[index] = descriptorCount;
	}
}

#endif