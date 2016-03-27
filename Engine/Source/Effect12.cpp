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
		D3D12_SHADER_VISIBILITY_HULL,
		D3D12_SHADER_VISIBILITY_DOMAIN,
		D3D12_SHADER_VISIBILITY_GEOMETRY,
		D3D12_SHADER_VISIBILITY_PIXEL
	};

	const Kodiak::ShaderType s_shaderStage[] =
	{
		ShaderType::Vertex,
		ShaderType::Hull,
		ShaderType::Domain,
		ShaderType::Geometry,
		ShaderType::Pixel
	};

} // anonymous namespace


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


static void whoa() {}
void Effect::BuildEffectSignature()
{
	// Clear out old effect signature data (shouldn't have any, but just to be safe...)
	m_signature.perViewDataSize = 0;
	m_signature.perObjectDataSize = 0;
	m_signature.perViewDataIndex = kInvalid;
	m_signature.perObjectDataIndex = kInvalid;

	// Create the root signature.  We'll populate the root parameters as we go through the shaders.
	CreateRootSignature();

	// Process shaders
	uint32_t rootIndex = 0;
	ProcessShaderBindings(rootIndex, m_vertexShader.get());
	ProcessShaderBindings(rootIndex, m_hullShader.get());
	ProcessShaderBindings(rootIndex, m_domainShader.get());
	ProcessShaderBindings(rootIndex, m_geometryShader.get());
	ProcessShaderBindings(rootIndex, m_pixelShader.get());

	whoa();

	// Finalize root signature
	m_rootSig->Finalize();
#if 0
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
#endif
}


void Effect::BuildPSO()
{
#if 0
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
#endif
}


void Effect::CreateRootSignature()
{
	uint32_t numRootParameters = 0;
	uint32_t numRootSamplers = 0;
	bool usePerViewData = false;
	bool usePerObjectData = false;

	if (m_vertexShader)
	{
		const auto& sig = m_vertexShader->GetSignature();
		auto vsDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (vsDescriptors > 6) ? 1 : vsDescriptors;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || (sig.cbvPerViewData.sizeInBytes != 0);
		usePerObjectData = usePerObjectData || (sig.cbvPerObjectData.sizeInBytes != 0);
	}

	if (m_hullShader)
	{
		const auto& sig = m_hullShader->GetSignature();
		auto hsDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (hsDescriptors > 6) ? 1 : hsDescriptors;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || (sig.cbvPerViewData.sizeInBytes != 0);
		usePerObjectData = usePerObjectData || (sig.cbvPerObjectData.sizeInBytes != 0);
	}

	if (m_domainShader)
	{
		const auto& sig = m_domainShader->GetSignature();
		auto dsDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (dsDescriptors > 6) ? 1 : dsDescriptors;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || (sig.cbvPerViewData.sizeInBytes != 0);
		usePerObjectData = usePerObjectData || (sig.cbvPerObjectData.sizeInBytes != 0);
	}

	if (m_geometryShader)
	{
		const auto& sig = m_geometryShader->GetSignature();
		auto gsDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (gsDescriptors > 6) ? 1 : gsDescriptors;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || (sig.cbvPerViewData.sizeInBytes != 0);
		usePerObjectData = usePerObjectData || (sig.cbvPerObjectData.sizeInBytes != 0);
	}

	if (m_pixelShader)
	{
		const auto& sig = m_pixelShader->GetSignature();
		auto psDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (psDescriptors > 6) ? 1 : psDescriptors;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || (sig.cbvPerViewData.sizeInBytes != 0);
		usePerObjectData = usePerObjectData || (sig.cbvPerObjectData.sizeInBytes != 0);
	}

	if (usePerViewData)
	{
		numRootParameters++;
	}

	if (usePerObjectData)
	{
		numRootParameters++;
	}

	m_rootSig = make_shared<RootSignature>(numRootParameters, numRootSamplers);
}


void Effect::ProcessShaderBindings(uint32_t& rootIndex, Shader* shader)
{
	using namespace ShaderReflection;

	if (!shader)
	{
		return;
	}

	const auto& shaderSig = shader->GetSignature();
	const auto shaderType = shader->GetType();
	const uint32_t shaderIndex = static_cast<uint32_t>(shaderType);

	auto& rootSig = *m_rootSig;

	// Validate per-view data size - must be zero or the same as the other shaders in the effect
	auto perViewDataSize = shader->GetPerViewDataSize();
	if (perViewDataSize > 0 && perViewDataSize != kInvalid)
	{
		assert((m_signature.perViewDataSize == 0) || (m_signature.perViewDataSize == perViewDataSize));
		m_signature.perViewDataSize = perViewDataSize;

		if (m_signature.perViewDataIndex == kInvalid)
		{
			m_signature.perViewDataIndex = m_signature.totalDescriptors++;

			// Setup root parameter
			rootSig[rootIndex++].InitAsConstantBuffer(GetPerViewConstantsSlot());
			m_signature.rootParameters.push_back(DescriptorRange(m_signature.perViewDataIndex));
		}
	}

	// Validate per-object data size - must be zero or the same as the other shaders in the effect
	auto perObjectDataSize = shader->GetPerObjectDataSize();
	if (perObjectDataSize > 0 && perObjectDataSize != kInvalid)
	{
		assert((m_signature.perObjectDataSize == 0) || (m_signature.perObjectDataSize == perObjectDataSize));
		m_signature.perObjectDataSize = perObjectDataSize;

		if (m_signature.perObjectDataIndex == kInvalid)
		{
			m_signature.perObjectDataIndex = m_signature.totalDescriptors++;

			// Setup root parameter
			rootSig[rootIndex++].InitAsConstantBuffer(GetPerObjectConstantsSlot());
			m_signature.rootParameters.push_back(DescriptorRange(m_signature.perObjectDataIndex));
		}
	}

	const bool useTable = shaderSig.numMaterialDescriptors > 6;

	if (useTable)
	{
		// TODO Setup descriptor tables here
	}

	// Constant buffers
	for (const auto& cbv : shaderSig.cbvTable)
	{
		if (!useTable)
		{
			// Create root parameter
			rootSig[rootIndex++].InitAsConstantBuffer(cbv.shaderRegister, s_shaderVisibility[shaderIndex]);
		}

		// Master descriptor array index
		CBVData cbvData;
		cbvData.descriptorTableSlot = m_signature.totalDescriptors++;

		// TODO: memory mapping

		m_signature.cbvMappingData.push_back(cbvData);
		m_signature.rootParameters.push_back(DescriptorRange(cbvData.descriptorTableSlot));
	}

	// SRVs
	for (const auto& srv : shaderSig.resources)
	{
		if (!useTable)
		{
			// Create root parameter
			rootSig[rootIndex++].InitAsBufferSRV(srv.shaderRegister[0], s_shaderVisibility[shaderIndex]);
		}
		const auto index = m_signature.totalDescriptors++;

		// See if we already have this SRV resource from a previous shader stage
		bool newResource = true;
		for (auto& fxSrv : m_signature.srvs)
		{
			if (fxSrv.name == srv.name)
			{
				// Confirm that the pre-existing SRV resource matches the new one
				assert(fxSrv.type == srv.type);
				assert(fxSrv.dimension == srv.dimension);
				fxSrv.binding[shaderIndex].tableIndex = index;
				fxSrv.binding[shaderIndex].tableSlot = kInvalid;

				if (!useTable)
				{
					m_signature.rootParameters.push_back(DescriptorRange(index));
				}
			}
		}

		// Create new SRV resource
		if (newResource)
		{
			ShaderReflection::ResourceSRV<5> fxSrv(srv);
			fxSrv.binding[shaderIndex].tableIndex = index;
			fxSrv.binding[shaderIndex].tableSlot = kInvalid;

			m_signature.srvs.push_back(fxSrv);

			if (!useTable)
			{
				m_signature.rootParameters.push_back(DescriptorRange(index));
			}
		}
	}

	// UAVs
	for (const auto& uav : shaderSig.uavs)
	{
		if (!useTable)
		{
			// Create root parameter
			rootSig[rootIndex++].InitAsBufferUAV(uav.shaderRegister[0], s_shaderVisibility[shaderIndex]);
		}
		const auto index = m_signature.totalDescriptors++;

		// See if we already have this SRV resource from a previous shader stage
		bool newResource = true;
		for (auto& fxUav : m_signature.uavs)
		{
			if (fxUav.name == uav.name)
			{
				// Confirm that the pre-existing SRV resource matches the new one
				assert(fxUav.type == uav.type);
				fxUav.binding[shaderIndex].tableIndex = index;
				fxUav.binding[shaderIndex].tableSlot = kInvalid;

				if (!useTable)
				{
					m_signature.rootParameters.push_back(DescriptorRange(index));
				}
			}
		}

		// Create new UAV resource
		if (newResource)
		{
			ShaderReflection::ResourceUAV<5> fxUav(uav);
			fxUav.binding[shaderIndex].tableIndex = index;
			fxUav.binding[shaderIndex].tableSlot = kInvalid;

			m_signature.uavs.push_back(fxUav);

			if (!useTable)
			{
				m_signature.rootParameters.push_back(DescriptorRange(index));
			}
		}
	}

	// TODO samplers
}


#if 0
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