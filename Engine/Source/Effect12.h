// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder

#pragma once

#include "ShaderReflection.h"

namespace Kodiak
{

// Forward declarations
class GraphicsPSO;
class RootSignature;
class Shader;
enum class ShaderType;


#if 0
struct EffectConstantBufferDesc
{
	std::string name;
	uint32_t	rootParameterIndex;
	uint32_t	rootTableOffset;
	uint32_t	shaderRegister;
	size_t		size;
	ShaderType	shaderStage;
};


struct EffectResourceBinding
{
	uint32_t	rootParameterIndex{ 0xFFFFFFFF };
	uint32_t	rootTableOffset{ 0xFFFFFFFF };
};


struct EffectResourceDesc
{
	ShaderResourceType						type{ ShaderResourceType::Unsupported };
	std::array<EffectResourceBinding, 5>	bindings;
};


struct EffectSignature
{
	EffectSignature()
		: perViewDataSize(0)
		, perObjectDataSize(0)
		, perShaderDescriptorCount({ 0, 0, 0, 0, 0 })
		, numRootParameters(0)
		, numStaticSamplers(0)
	{}

	// Shared constants for multiple shader stages
	size_t	perViewDataSize;
	size_t	perObjectDataSize;

	// Number of material descriptors (CBV, SRV, UAV) per shader stage
	std::array<size_t, 5> perShaderDescriptorCount;

	// Total number of root parameters in RootSignature
	size_t	numRootParameters;
	// Total number of static samplers in RootSignature
	size_t	numStaticSamplers;

	// List of constant buffer descriptions
	std::vector<EffectConstantBufferDesc> cbuffers;

	// Map of resources
	std::map<std::string, EffectResourceDesc>	resources;
};
#endif


class Effect : public BaseEffect
{
public:
	// Forward decls
	struct Signature;

	Effect();
	explicit Effect(const std::string& name);

	const Signature& GetSignature() const { return m_signature; }
	std::shared_ptr<GraphicsPSO> GetPSO() { return m_pso; }
	std::shared_ptr<RootSignature> GetRootSignature() { return m_rootSig; }

	void Finalize() override;

	struct CBVData
	{
		uint32_t descriptorTableSlot{ kInvalid };
	};

	struct Signature
	{
		// Per-view and per-object CBV bindings
		uint32_t	perViewDataIndex{ kInvalid };
		uint32_t	perObjectDataIndex{ kInvalid };
		uint32_t	perViewDataSize{ 0 };   // For validation
		uint32_t	perObjectDataSize{ 0 }; // For validation

		// Count of total CPU descriptors in the master array
		uint32_t			totalDescriptors{ 0 };

		// Root parameters
		std::vector<ShaderReflection::DescriptorRange> rootParameters;

		// Descriptor tables
		std::vector<uint32_t> cbvDescriptorMap;

		// Mapping data
		std::vector<CBVData> cbvMappingData;
		std::vector<ShaderReflection::ResourceSRV<5>> srvs;
		std::vector<ShaderReflection::ResourceUAV<5>> uavs;
	};

private:
	void BuildEffectSignature();
	void BuildPSO();

	void CreateRootSignature();
	void ProcessShaderBindings(uint32_t& rootIndex, Shader* shader);

#if 0
	void BuildConstantBufferDesc(const ShaderConstantBufferDesc& desc, uint32_t rootParameterIndex, uint32_t rootTableOffset,
		ShaderType shaderType);
	void BuildResourceDesc(const ShaderResourceDesc& desc, uint32_t rootParameterIndex, uint32_t rootTableOffset, uint32_t shaderIndex);
	void ProcessShaderBindings(uint32_t index, Shader* shader);
#endif
	

private:
	std::shared_ptr<RootSignature>	m_rootSig;
	std::shared_ptr<GraphicsPSO>	m_pso;

	Signature						m_signature;
};

} // namespace Kodiak