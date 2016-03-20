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
class BaseShader;
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


struct EffectSignature
{
	// Shared constants for multiple shader stages
	size_t	perViewDataSize;
	size_t	perObjectDataSize;
	size_t  perMaterialDataSize;

	// Constant buffers
	std::array<std::vector<EffectConstantBuffer>, 5> constantBuffers;

	// Resources
	std::vector<EffectResource> resources;

	std::vector<EffectVariable>	variables;
};


class Effect : public BaseEffect
{
public:
	Effect();
	explicit Effect(const std::string& name);

	const EffectSignature& GetSignature() const { return m_signature; }
	std::shared_ptr<GraphicsPSO> GetPSO() { return m_pso; }

	void Finalize() override;

private:
	void BuildEffectSignature();
	void BuildPSO();
	template <class ShaderClass>
	void ProcessShaderBindings(uint32_t index, ShaderClass* shader);

private:
	std::shared_ptr<GraphicsPSO>	m_pso;
	EffectSignature					m_signature;
};

} // namespace Kodiak
