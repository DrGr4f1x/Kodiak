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

#include "CommonStates.h"
#include "InputLayout12.h"
#include "Material.h"
#include "RenderEnums.h"
#include "RootSignature12.h"
#include "Shader.h"
#include "ShaderResource12.h"


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
	m_signature.perViewDataSize = 0;
	m_signature.perObjectDataSize = 0;
	m_signature.perViewDataIndex = kInvalid;
	m_signature.perObjectDataIndex = kInvalid;
	m_signature.cbvPerMaterialDataSize = 0;

	// Create the root signature.  We'll populate the root parameters as we go through the shaders.
	CreateRootSignature();

	// Process shaders
	uint32_t rootIndex = 0;
	ProcessShaderBindings(rootIndex, m_vertexShader.get());
	ProcessShaderBindings(rootIndex, m_hullShader.get());
	ProcessShaderBindings(rootIndex, m_domainShader.get());
	ProcessShaderBindings(rootIndex, m_geometryShader.get());
	ProcessShaderBindings(rootIndex, m_pixelShader.get());

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

	// Finalize root signature
	auto flags = GetRootSignatureFlags();
	m_rootSig->Finalize(flags);
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
	
	auto resource = m_vertexShader->GetResource();
	m_pso->SetInputLayout(resource->GetInputLayout());

	m_pso->SetVertexShader(m_vertexShader.get());
	m_pso->SetDomainShader(m_domainShader.get());
	m_pso->SetHullShader(m_hullShader.get());
	m_pso->SetGeometryShader(m_geometryShader.get());
	m_pso->SetPixelShader(m_pixelShader.get());

	m_pso->Finalize();
}


namespace
{

inline bool IsValidSize(uint32_t size)
{
	return (size != 0) && (size != kInvalid);
}

} // anonymous namespace


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
		numRootParameters += (vsDescriptors > 0) ? 1 : 0;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || IsValidSize(sig.cbvPerViewData.sizeInBytes);
		usePerObjectData = usePerObjectData || IsValidSize(sig.cbvPerObjectData.sizeInBytes);
	}

	if (m_hullShader)
	{
		const auto& sig = m_hullShader->GetSignature();
		auto hsDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (hsDescriptors > 0) ? 1 : 0;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || IsValidSize(sig.cbvPerViewData.sizeInBytes);
		usePerObjectData = usePerObjectData || IsValidSize(sig.cbvPerObjectData.sizeInBytes);
	}

	if (m_domainShader)
	{
		const auto& sig = m_domainShader->GetSignature();
		auto dsDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (dsDescriptors > 0) ? 1 : 0;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || IsValidSize(sig.cbvPerViewData.sizeInBytes);
		usePerObjectData = usePerObjectData || IsValidSize(sig.cbvPerObjectData.sizeInBytes);
	}

	if (m_geometryShader)
	{
		const auto& sig = m_geometryShader->GetSignature();
		auto gsDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (gsDescriptors > 0) ? 1 : 0;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || IsValidSize(sig.cbvPerViewData.sizeInBytes);
		usePerObjectData = usePerObjectData || IsValidSize(sig.cbvPerObjectData.sizeInBytes);
	}

	if (m_pixelShader)
	{
		const auto& sig = m_pixelShader->GetSignature();
		auto psDescriptors = sig.numMaterialDescriptors;
		numRootParameters += (psDescriptors > 0) ? 1 : 0;
		numRootSamplers += sig.numSamplers;

		usePerViewData = usePerViewData || IsValidSize(sig.cbvPerViewData.sizeInBytes);
		usePerObjectData = usePerObjectData || IsValidSize(sig.cbvPerObjectData.sizeInBytes);
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


void Effect::ProcessShaderBindings(uint32_t& rootIndex, IShader* shader)
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
			const auto curRootIndex = rootIndex;
			rootSig[rootIndex++].InitAsConstantBuffer(GetPerViewConstantsSlot());
			m_signature.rootParameters.push_back(DescriptorRange(curRootIndex, m_signature.perViewDataIndex));
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
			const auto curRootIndex = rootIndex;
			rootSig[rootIndex++].InitAsConstantBuffer(GetPerObjectConstantsSlot());
			m_signature.rootParameters.push_back(DescriptorRange(curRootIndex, m_signature.perObjectDataIndex));
		}
	}

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

	// Setup descriptor tables
	if (shaderSig.numMaterialDescriptors > 0)
	{
		// Count ranges
		uint32_t numRanges = 0;
		numRanges += static_cast<uint32_t>(shaderSig.cbvTable.size());
		numRanges += static_cast<uint32_t>(shaderSig.srvTable.size());
		numRanges += static_cast<uint32_t>(shaderSig.uavTable.size());

		// Create root parameter for the table
		const auto curRootIndex = rootIndex;
		auto& rootParam = rootSig[rootIndex++];
		rootParam.InitAsDescriptorTable(numRanges, s_shaderVisibility[shaderIndex]);

		uint32_t rangeIndex = 0;

		// Constant buffers
		m_signature.cbvBindings[shaderIndex] = shaderSig.cbvTable;
		for (auto& cbv : m_signature.cbvBindings[shaderIndex])
		{
			rootParam.SetTableRange(rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, cbv.shaderRegister, 1);

			cbv.binding.tableIndex = m_signature.totalDescriptors++;
			cbv.binding.tableSlot = kInvalid;

			m_signature.rootParameters.push_back(DescriptorRange(curRootIndex, cbv.binding.tableIndex, 1));

			++rangeIndex;
		}

		// SRVs
		for (const auto& srvTable : shaderSig.srvTable)
		{
			rootParam.SetTableRange(rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvTable.shaderRegister, srvTable.numItems);

			const auto index = m_signature.totalDescriptors;
			const auto numItems = srvTable.numItems;
			m_signature.rootParameters.push_back(DescriptorRange(curRootIndex, index, numItems));
			m_signature.totalDescriptors += numItems;

			// Bind SRVs (material parameters) here
			const uint32_t firstRegister = srvTable.shaderRegister;
			const uint32_t lastRegister = firstRegister + srvTable.numItems;
			uint32_t currentDescriptorSlot = index;

			// Loop over the shader registers included in this table
			for (uint32_t shaderRegister = firstRegister; shaderRegister < lastRegister; ++shaderRegister)
			{
				bool found = false;
				// Find the SRV from the shader mapped to the current shader register
				for (const auto& srv : shaderSig.resources)
				{
					if (srv.shaderRegister[0] == shaderRegister)
					{
						auto it = m_signature.srvs.find(srv.name);
						if (end(m_signature.srvs) == it)
						{
							ShaderReflection::ResourceSRV<5> fxSrv(shaderIndex, srv);
							fxSrv.binding[shaderIndex].tableIndex = currentDescriptorSlot;
							fxSrv.binding[shaderIndex].tableSlot = kInvalid;

							m_signature.srvs[srv.name] = fxSrv;
						}
						else
						{
							auto& fxSrv = it->second;

							// Confirm that the pre-existing SRV resource matches the new one
							assert(fxSrv.type == srv.type);
							assert(fxSrv.dimension == srv.dimension);
							fxSrv.binding[shaderIndex].tableIndex = currentDescriptorSlot;
							fxSrv.binding[shaderIndex].tableSlot = kInvalid;
						}

						found = true;
						break;
					}
				}
				++currentDescriptorSlot;
				assert(found);
			}

			++rangeIndex;
		}

		// UAVs
		for (const auto& uavTable : shaderSig.uavTable)
		{
			rootParam.SetTableRange(rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uavTable.shaderRegister, uavTable.numItems);

			const auto index = m_signature.totalDescriptors;
			const auto numItems = uavTable.numItems;
			m_signature.rootParameters.push_back(DescriptorRange(curRootIndex, index, numItems));
			m_signature.totalDescriptors += numItems;

			// Bind UAVs (material parameters) here
			const uint32_t firstRegister = uavTable.shaderRegister;
			const uint32_t lastRegister = firstRegister + uavTable.numItems;
			uint32_t currentDescriptorSlot = index;

			// Loop over the shader registers included in this table
			for (uint32_t shaderRegister = firstRegister; shaderRegister < lastRegister; ++shaderRegister)
			{
				bool found = false;
				// Find the UAV from the shader mapped to the current shader register
				for (const auto& uav : shaderSig.uavs)
				{
					if (uav.shaderRegister[0] == shaderRegister)
					{
						auto it = m_signature.uavs.find(uav.name);
						if (end(m_signature.uavs) == it)
						{
							ShaderReflection::ResourceUAV<5> fxUav(shaderIndex, uav);
							fxUav.binding[shaderIndex].tableIndex = currentDescriptorSlot;
							fxUav.binding[shaderIndex].tableSlot = kInvalid;

							m_signature.uavs[uav.name] = fxUav;
						}
						else
						{
							auto& fxUav = it->second;

							// Confirm that the pre-existing UAV resource matches the new one
							assert(fxUav.type == uav.type);
							fxUav.binding[shaderIndex].tableIndex = currentDescriptorSlot;
							fxUav.binding[shaderIndex].tableSlot = kInvalid;
						}

						found = true;
						break;
					}
				}
				++currentDescriptorSlot;
				assert(found);
			}

			++rangeIndex;
		}
	}

	// Samplers
	for (const auto& sampler : shaderSig.samplers)
	{
		const auto& samplerDesc = CommonStates::NamedSampler(sampler.name);

		rootSig.InitStaticSampler(sampler.shaderRegister[0], samplerDesc, s_shaderVisibility[shaderIndex]);
	}
}


void Effect::TryWaitShader(IShader* shader)
{
	if (shader)
	{
		shader->Wait();
	}
}


D3D12_ROOT_SIGNATURE_FLAGS Effect::GetRootSignatureFlags()
{
	D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	auto resource = m_vertexShader->GetResource();
	if (!resource->GetInputLayout().elements.empty())
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	}

	if (m_vertexShader->GetSignature().numDescriptors == 0)
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	}

	if (!m_hullShader || (m_hullShader && (m_hullShader->GetSignature().numDescriptors == 0)))
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	}

	if (!m_domainShader || (m_domainShader && (m_domainShader->GetSignature().numDescriptors == 0)))
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	}

	if (!m_geometryShader || (m_geometryShader && (m_geometryShader->GetSignature().numDescriptors == 0)))
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	}

	if (!m_pixelShader || (m_pixelShader && (m_pixelShader->GetSignature().numDescriptors == 0)))
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	}

	return flags;
}