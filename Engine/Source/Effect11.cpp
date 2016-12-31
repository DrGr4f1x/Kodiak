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

#include "PipelineState11.h"
#include "Shader.h"
#include "ShaderResource11.h"


using namespace Kodiak;
using namespace std;


Effect::Effect() : BaseEffect() {}


Effect::Effect(const string& name) : BaseEffect(name) {}


void Effect::Finalize()
{
	if (m_isFinalized) return;

	TryWaitShader(m_vertexShader.get());
	TryWaitShader(m_hullShader.get());
	TryWaitShader(m_domainShader.get());
	TryWaitShader(m_geometryShader.get());
	TryWaitShader(m_pixelShader.get());

	BuildEffectSignature();
	BuildPSO();
	
	m_isFinalized = true;
}


void Effect::BuildEffectSignature()
{
	// Clear out old effect signature data (shouldn't have any, but just to be safe...)
	for (uint32_t i = 0; i < 5; ++i)
	{
		m_signature.perViewDataBindings[i] = kInvalid;
		m_signature.perObjectDataBindings[i] = kInvalid;
	}
	m_signature.perViewDataSize = 0;
	m_signature.perObjectDataSize = 0;
	m_signature.cbvPerMaterialDataSize = 0;

	// Process the shaders
	ProcessShaderBindings(m_vertexShader.get());
	ProcessShaderBindings(m_domainShader.get());
	ProcessShaderBindings(m_hullShader.get());
	ProcessShaderBindings(m_geometryShader.get());
	ProcessShaderBindings(m_pixelShader.get());

	// Patch the constant buffer offsets 
	// (number of bytes from the start of the uber-cbuffer for each sub-buffer)
	uint32_t currentOffset = 0;
	for (uint32_t i = 0; i < 5; ++i)
	{
		// Patch cbuffer offsets (for logical sub-buffers)
		for (auto& cbvBinding : m_signature.cbvBindings[i])
		{
			if (cbvBinding.byteOffset == 0)
			{
				// Verify that the cbuffer has a legitimate size
				assert(cbvBinding.sizeInBytes != kInvalid);
				assert(cbvBinding.sizeInBytes != 0);

				cbvBinding.byteOffset = currentOffset;
				currentOffset += cbvBinding.sizeInBytes;
				m_signature.cbvPerMaterialDataSize += cbvBinding.sizeInBytes;
			}
		}
	}

	// Patch the parameter offsets
	// (number of bytes from the start of the uber-cbuffer for each sub-buffer)
	for (auto& parameter : m_signature.parameters)
	{
		for (uint32_t i = 0; i < 5; ++i)
		{
			if (parameter.second.byteOffset[i] != kInvalid)
			{
				const auto cbvShaderRegister = parameter.second.cbvShaderRegister[i];

				for (const auto& cbvBinding : m_signature.cbvBindings[i])
				{
					if (cbvBinding.shaderRegister == cbvShaderRegister)
					{
						assert(cbvBinding.byteOffset != kInvalid);
						parameter.second.byteOffset[i] += cbvBinding.byteOffset;
						break;
					}
				}

				assert(parameter.second.byteOffset[i] != kInvalid);
			}
		}
	}
}


void Effect::BuildPSO()
{
	m_pso = make_shared<GraphicsPSO>();

	assert(m_vertexShader.get()); // Vertex shader is mandatory
	m_pso->SetVertexShader(m_vertexShader.get());
	auto resource = m_vertexShader->GetResource();
	m_pso->SetInputLayout(resource->GetInputLayout());

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


void Effect::ProcessShaderBindings(IShader* shader)
{
	using namespace ShaderReflection;

	if (!shader)
	{
		return;
	}

	const auto& shaderSig = shader->GetSignature();
	const auto shaderType = shader->GetType();
	const uint32_t shaderIndex = static_cast<uint32_t>(shaderType);

	// Validate per-view data size - must be zero or the same as the other shaders in the effect
	auto perViewDataSize = shader->GetPerViewDataSize();
	if (perViewDataSize > 0 && perViewDataSize != kInvalid)
	{
		assert((m_signature.perViewDataSize == 0) || (m_signature.perViewDataSize == perViewDataSize));
		m_signature.perViewDataSize = perViewDataSize;

		m_signature.perViewDataBindings[shaderIndex] = shaderSig.cbvPerViewData.shaderRegister;
	}

	// Validate per-object data size - must be zero or the same as the other shaders in the effect
	auto perObjectDataSize = shader->GetPerObjectDataSize();
	if (perObjectDataSize > 0 && perObjectDataSize != kInvalid)
	{
		assert((m_signature.perObjectDataSize == 0) || (m_signature.perObjectDataSize == perObjectDataSize));
		m_signature.perObjectDataSize = perObjectDataSize;

		m_signature.perObjectDataBindings[shaderIndex] = shaderSig.cbvPerObjectData.shaderRegister;
	}

	// Copy CBV tables
	m_signature.cbvBindings[shaderIndex] = shaderSig.cbvTable;
	
	// Copy SRV tables
	m_signature.srvBindings[shaderIndex] = shaderSig.srvTable;
	
	// Copy UAV tables
	m_signature.uavBindings[shaderIndex] = shaderSig.uavTable;
	
	// Copy sampler tables
	m_signature.samplerBindings[shaderIndex] = shaderSig.samplerTable;
	

	// Parameters
	for (const auto& parameter : shaderSig.parameters)
	{
		auto it = m_signature.parameters.find(parameter.name);
		if (end(m_signature.parameters) == it)
		{
			m_signature.parameters[parameter.name] = Parameter<5>(shaderIndex, parameter);
		}
		else
		{
			it->second.Assign(shaderIndex, parameter);
		}
	}

	// SRV resources
	for (const auto& srv : shaderSig.resources)
	{
		auto it = m_signature.srvs.find(srv.name);
		if (end(m_signature.srvs) == it)
		{
			m_signature.srvs[srv.name] = ResourceSRV<5>(shaderIndex, srv);
		}
		else
		{
			it->second.Assign(shaderIndex, srv);
		}
	}

	// UAV resources
	for (const auto& uav : shaderSig.uavs)
	{
		auto it = m_signature.uavs.find(uav.name);
		if (end(m_signature.uavs) == it)
		{
			m_signature.uavs[uav.name] = ResourceUAV<5>(shaderIndex, uav);
		}
		else
		{
			it->second.Assign(shaderIndex, uav);
		}
	}

	// Samplers
	for (const auto& sampler : shaderSig.samplers)
	{
		auto it = m_signature.samplers.find(sampler.name);
		if (end(m_signature.samplers) == it)
		{
			m_signature.samplers[sampler.name] = Sampler<5>(shaderIndex, sampler);
		}
		else
		{
			it->second.Assign(shaderIndex, sampler);
		}
	}
}


void Effect::TryWaitShader(IShader* shader)
{
	if (shader)
	{
		shader->Wait();
	}
}