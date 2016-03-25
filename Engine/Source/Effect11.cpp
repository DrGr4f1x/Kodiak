// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Effect.h"

#include "MathUtil.h"
#include "PipelineState11.h"
#include "Shader11.h"


using namespace Kodiak;
using namespace std;


Effect::Effect() : BaseEffect() {}


Effect::Effect(const string& name) : BaseEffect(name) {}


void Effect::Finalize()
{
	loadTask = loadTask.then([this]
	{
		BuildEffectSignature();
		BuildPSO();
	});
}


void Effect::BuildEffectSignature()
{
	// Clear out old effect signature data (shouldn't have any, but just to be safe...)
	m_signature.perViewDataSize = 0;
	m_signature.perObjectDataSize = 0;
	m_signature.perMaterialDataSize = 0;

	// Process the shaders
	ProcessShaderBindings(0, m_vertexShader.get());
	ProcessShaderBindings(1, m_domainShader.get());
	ProcessShaderBindings(2, m_hullShader.get());
	ProcessShaderBindings(3, m_geometryShader.get());
	ProcessShaderBindings(4, m_pixelShader.get());
}


void Effect::BuildPSO()
{
	m_pso = make_shared<GraphicsPSO>();

	assert(m_vertexShader.get()); // Vertex shader is mandatory
	m_pso->SetVertexShader(m_vertexShader.get());
	m_pso->SetInputLayout(*m_vertexShader->GetInputLayout());

	if (m_domainShader)
	{
		m_pso->SetDomainShader(m_domainShader.get());
	}

	if (m_hullShader)
	{
		m_pso->SetHullShader(m_hullShader.get());
	}

	if (m_geometryShader)
	{
		m_pso->SetGeometryShader(m_geometryShader.get());
	}

	if (m_pixelShader)
	{
		m_pso->SetPixelShader(m_pixelShader.get());
	}

	m_pso->SetBlendState(m_blendStateDesc);
	m_pso->SetRasterizerState(m_rasterizerStateDesc);
	m_pso->SetDepthStencilState(m_depthStencilStateDesc);

	m_pso->Finalize();
}


template <class ShaderClass>
void Effect::ProcessShaderBindings(uint32_t index, ShaderClass* shader)
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

		const auto& shaderSig = shader->GetSignature();

		// Constant buffers
		for (const auto& shaderCBuffer : shaderSig.cbuffers)
		{
			EffectConstantBuffer effectCBuffer;
			effectCBuffer.name = shaderCBuffer.name;
			effectCBuffer.size = shaderCBuffer.size;
			effectCBuffer.shaderRegister = shaderCBuffer.registerSlot;
			m_signature.internalCBVToDXMap[index].emplace_back(effectCBuffer);

			m_signature.perMaterialDataSize += Math::AlignUp(shaderCBuffer.size, 16);
		}

		// Resources
		std::vector<uint32_t> resourceSlots;
		for (const auto& shaderResource : shaderSig.resources)
		{
			bool found = false;
			for (auto& effectResource : m_signature.resources)
			{
				if (effectResource.name == shaderResource.name)
				{
					assert(effectResource.type == shaderResource.type);
					assert(effectResource.dimension == shaderResource.dimension);
					effectResource.shaderSlots[index] = shaderResource.registerSlot;
					found = true;
					break;
				}
			}

			if (!found)
			{
				EffectResource effectResource;
				effectResource.name = shaderResource.name;
				fill(begin(effectResource.shaderSlots), end(effectResource.shaderSlots), 0xFFFFFFFF);
				effectResource.shaderSlots[index] = shaderResource.registerSlot;
				effectResource.type = shaderResource.type;
				effectResource.dimension = shaderResource.dimension;
				m_signature.resources.emplace_back(effectResource);
			}

			// Store slots so we can build resource ranges
			resourceSlots.push_back(shaderResource.registerSlot);
		}

		// Build resource ranges
		if (!resourceSlots.empty())
		{
			sort(begin(resourceSlots), end(resourceSlots));

			EffectShaderResourceBinding::ResourceRange range = { resourceSlots[0], 1 };
			const size_t numSlots = resourceSlots.size();
			for (size_t i = 1; i < numSlots; ++i)
			{
				if (resourceSlots[i] == range.startSlot + range.numResources)
				{
					++range.numResources;
				}
				else
				{
					m_signature.internalSRVToDXMap[index].resourceRanges.push_back(range);
					range.startSlot = resourceSlots[i];
					range.numResources = 1;
				}
			}
			m_signature.internalSRVToDXMap[index].resourceRanges.push_back(range);
		}


		// Variables
		const auto& shaderVariables = shader->GetVariables();
		for (const auto& shaderVariable : shaderVariables)
		{
			bool found = false;
			for (auto& effectVariable : m_signature.variables)
			{
				if (effectVariable.name == shaderVariable.name)
				{
					assert(effectVariable.size == shaderVariable.size);
					assert(effectVariable.type == shaderVariable.type);
					effectVariable.shaderSlots[index].cbufferIndex = shaderVariable.constantBuffer;
					effectVariable.shaderSlots[index].offset = shaderVariable.startOffset;
					found = true;
					break;
				}
			}

			if (!found)
			{
				EffectVariable effectVariable;
				effectVariable.name = shaderVariable.name;
				effectVariable.size = shaderVariable.size;
				effectVariable.type = shaderVariable.type;
				fill(begin(effectVariable.shaderSlots), end(effectVariable.shaderSlots), EffectVariableBinding{ 0xFFFFFFFF, 0xFFFFFFFF });
				effectVariable.shaderSlots[index].cbufferIndex = shaderVariable.constantBuffer;
				effectVariable.shaderSlots[index].offset = shaderVariable.startOffset;
				m_signature.variables.emplace_back(effectVariable);
			}
		}
	}
}