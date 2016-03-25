// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
class GraphicsPSO;


struct EffectConstantBuffer
{
	std::string name;
	uint32_t	shaderRegister;
	size_t		size;
};


struct EffectResource
{
	std::string				name;
	std::array<uint32_t, 5> shaderSlots;
	ShaderResourceType		type;
	ShaderResourceDimension dimension;
};

struct EffectShaderResourceBinding
{
	struct ResourceRange
	{
		uint32_t startSlot;
		uint32_t numResources;
	};
	std::vector<ResourceRange> resourceRanges;
};


struct EffectVariableBinding
{
	uint32_t cbufferIndex;
	uint32_t offset;
};


struct EffectVariable
{
	std::string								name;
	size_t									size;
	ShaderVariableType						type;
	std::array<EffectVariableBinding, 5>	shaderSlots;
};


class Effect : public BaseEffect
{
public:
	// Forward decls
	struct Signature;

	Effect();
	explicit Effect(const std::string& name);

	const Signature& GetSignature() const { return m_signature; }
	std::shared_ptr<GraphicsPSO> GetPSO() { return m_pso; }

	void Finalize() override;

	struct Signature
	{
		// Shared constants for multiple shader stages
		size_t	perViewDataSize;
		size_t	perObjectDataSize;
		size_t  perMaterialDataSize;

		// Remap internal CBVs to DX11 
		std::array<std::vector<EffectConstantBuffer>, 5>	internalCBVToDXMap;
		// Remap internal SRVs to DX11
		std::array<EffectShaderResourceBinding, 5>			internalSRVToDXMap;
		// TODO: internal UAVs to DX11

		// Application SRV to internal remap
		std::vector<EffectResource> resources;
		// Application parameter to internal CPU memory
		std::vector<EffectVariable> variables;
		// TODO: Application UAV to internal remap
	};

private:
	void BuildEffectSignature();
	void BuildPSO();
	template <class ShaderClass>
	void ProcessShaderBindings(uint32_t index, ShaderClass* shader);

private:
	std::shared_ptr<GraphicsPSO>	m_pso;
	Signature						m_signature;
};

} // namespace Kodiak
