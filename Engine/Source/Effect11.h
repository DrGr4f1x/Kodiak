// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "ShaderReflection.h"

namespace Kodiak
{

// Forward declarations
class GraphicsPSO;
class Shader;
enum class ShaderType;


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
		// Per-view and per-object CBV bindings
		std::array<uint32_t, 5> perViewDataBindings;
		std::array<uint32_t, 5> perObjectDataBindings;
		uint32_t				perViewDataSize;	// For validation
		uint32_t				perObjectDataSize;	// For validation

		// Per-material CBV bindings (one large cbuffer for everything)
		uint32_t												cbvPerMaterialDataSize{ kInvalid };
		std::array<std::vector<ShaderReflection::CBVLayout>, 5> cbvBindings;

		// Resource bindings
		std::array<std::vector<ShaderReflection::TableLayout>, 5> srvBindings;
		std::array<std::vector<ShaderReflection::TableLayout>, 5> uavBindings;
		std::array<std::vector<ShaderReflection::TableLayout>, 5> samplerBindings;

		// Parameters and resources
		std::vector<ShaderReflection::Parameter<5>>		parameters;
		std::vector<ShaderReflection::ResourceSRV<5>>	srvs;
		std::vector<ShaderReflection::ResourceUAV<5>>	uavs;
		std::vector<ShaderReflection::Sampler<5>>		samplers;
	};

private:
	void BuildEffectSignature();
	void BuildPSO();
	void ProcessShaderBindings(Shader* shader);

private:
	std::shared_ptr<GraphicsPSO>	m_pso;
	Signature						m_signature;
};

} // namespace Kodiak
