// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder

#pragma once

#if 0

namespace Kodiak
{

// Forward declarations
class BaseShader;
class GraphicsPSO;
class RootSignature;
enum class ShaderType;


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


class Effect : public BaseEffect
{
public:
	const EffectSignature& GetSignature() const { return m_signature; }

	void Finalize() override;

private:
	void BuildEffectSignature();
	void BuildRootSignature();
	void BuildPSO();

	void BuildConstantBufferDesc(const ShaderConstantBufferDesc& desc, uint32_t rootParameterIndex, uint32_t rootTableOffset,
		ShaderType shaderType);
	void BuildResourceDesc(const ShaderResourceDesc& desc, uint32_t rootParameterIndex, uint32_t rootTableOffset, uint32_t shaderIndex);
	void ProcessShaderBindings(uint32_t index, BaseShader* shader);
	

private:
	std::shared_ptr<RootSignature>	m_rootSig;
	std::shared_ptr<GraphicsPSO>	m_pso;

	EffectSignature					m_signature;
};

} // namespace Kodiak

#endif