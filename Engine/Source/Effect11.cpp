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
			if (parameter.byteOffsets[i] != kInvalid)
			{
				const auto cbvShaderRegister = parameter.cbvShaderRegister[i];

				for (const auto& cbvBinding : m_signature.cbvBindings[i])
				{
					if (cbvBinding.shaderRegister == cbvShaderRegister)
					{
						assert(cbvBinding.byteOffset != kInvalid);
						parameter.byteOffsets[i] += cbvBinding.byteOffset;
						break;
					}
				}

				assert(parameter.byteOffsets[i] != kInvalid);
			}
		}
	}
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


void Effect::ProcessShaderBindings(Shader* shader)
{
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

	// Constant buffers
	for (const auto& cbv : shaderSig.cbvTable)
	{
		CBVBinding binding;
		binding.byteOffset = 0;
		binding.sizeInBytes = cbv.sizeInBytes;
		binding.shaderRegister = cbv.shaderRegister;
		m_signature.cbvBindings[shaderIndex].push_back(binding);
	}

	// Copy SRV tables
	for (const auto& table : shaderSig.srvTable)
	{
		TableLayout layout;
		layout.shaderRegister = table.shaderRegister;
		layout.numItems = table.numItems;;
		m_signature.srvBindings[shaderIndex].push_back(layout);
	}

	// Copy UAV tables
	for (const auto& table : shaderSig.uavTable)
	{
		TableLayout layout;
		layout.shaderRegister = table.shaderRegister;
		layout.numItems = table.numItems; 
		m_signature.uavBindings[shaderIndex].push_back(layout);
	}

	// Copy sampler tables
	for (const auto& table : shaderSig.samplerTable)
	{
		TableLayout layout;
		layout.shaderRegister = table.shaderRegister;
		layout.numItems = table.numItems;
		m_signature.samplerBindings[shaderIndex].push_back(layout);
	}

	// Parameters
	for (const auto& parameter : shaderSig.parameters)
	{
		// See if we already have this parameter from a previous shader stage
		bool newParameter = true;
		for (auto& fxParameter : m_signature.parameters)
		{
			if (fxParameter.name == parameter.name)
			{
				// Confirm that the pre-existing parameter matches the new one
				assert(fxParameter.type == parameter.type);
				assert(fxParameter.sizeInBytes == parameter.sizeInBytes);
				fxParameter.byteOffsets[shaderIndex] = parameter.byteOffset; // Will be patched after all shaders are processed
				fxParameter.cbvShaderRegister[shaderIndex] = parameter.cbvShaderRegister;

				newParameter = false;
				break;
			}
		}

		// Create new parameter, if necessary
		if (newParameter)
		{
			Parameter fxParameter;
			fxParameter.name = parameter.name;
			fxParameter.type = parameter.type;
			fxParameter.sizeInBytes = parameter.sizeInBytes;
			fxParameter.byteOffsets[shaderIndex] = parameter.byteOffset; // Will be patched after all shaders are processed
			fxParameter.cbvShaderRegister[shaderIndex] = parameter.cbvShaderRegister;

			m_signature.parameters.push_back(fxParameter);
		}
	}

	// SRV resources
	for (const auto& srv : shaderSig.resources)
	{
		// See if we already have this SRV resource from a previous shader stage
		bool newResource = true;
		for (auto& fxSrv : m_signature.srvs)
		{
			if (fxSrv.name == srv.name)
			{
				// Confirm that the pre-existing SRV resource matches the new one
				assert(fxSrv.type == srv.type);
				assert(fxSrv.dimension == srv.dimension);
				fxSrv.bindings[shaderIndex].tableIndex = srv.tableIndex;
				fxSrv.bindings[shaderIndex].tableSlot = srv.tableSlot;

				newResource = false;
				break;
			}
		}

		// Create new SRV resource, if necessary
		if (newResource)
		{
			ResourceSRV fxSrv;
			fxSrv.name = srv.name;
			fxSrv.type = srv.type;
			fxSrv.dimension = srv.dimension;
			fxSrv.bindings[shaderIndex].tableIndex = srv.tableIndex;
			fxSrv.bindings[shaderIndex].tableSlot = srv.tableSlot;

			m_signature.srvs.push_back(fxSrv);
		}
	}

	// UAV resources
	for (const auto& uav : shaderSig.uavs)
	{
		// See if we already have this UAV resource from a previous shader stage
		bool newResource = true;
		for (auto& fxUav : m_signature.uavs)
		{
			if (fxUav.name == uav.name)
			{
				// Confirm that the pre-existing UAV resource matches the new one
				assert(fxUav.type == uav.type);
				fxUav.bindings[shaderIndex].tableIndex = uav.tableIndex;
				fxUav.bindings[shaderIndex].tableSlot = uav.tableSlot;

				newResource = false;
				break;
			}
		}

		// Create new UAV resource, if necessary
		if (newResource)
		{
			ResourceUAV fxUav;
			fxUav.name = uav.name;
			fxUav.type = uav.type;
			fxUav.bindings[shaderIndex].tableIndex = uav.tableIndex;
			fxUav.bindings[shaderIndex].tableSlot = uav.tableSlot;

			m_signature.uavs.push_back(fxUav);
		}
	}

	// Samplers
	for (const auto& sampler : shaderSig.samplers)
	{
		// See if we already have this sampler from a previous shader stage
		bool newSampler = true;
		for (auto& fxSampler : m_signature.samplers)
		{
			if (fxSampler.name == sampler.name)
			{
				// Nothing to validate for samplers
				fxSampler.bindings[shaderIndex].tableIndex = sampler.tableIndex;
				fxSampler.bindings[shaderIndex].tableSlot = sampler.tableSlot;

				newSampler = false;
				break;
			}
		}

		// Create new sampler, if necessary
		if (newSampler)
		{
			Sampler fxSampler;
			fxSampler.name = sampler.name;
			fxSampler.bindings[shaderIndex].tableIndex = sampler.tableIndex;
			fxSampler.bindings[shaderIndex].tableSlot = sampler.tableSlot;

			m_signature.samplers.push_back(fxSampler);
		}
	}
}