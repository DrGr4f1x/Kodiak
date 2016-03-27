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


class Effect : public BaseEffect
{
public:
	// Forward decls
	struct Signature;

	Effect();
	explicit Effect(const std::string& name);

	const Signature& GetSignature() const { return m_signature; }
	Signature& GetSignature() { return m_signature; }
	std::shared_ptr<GraphicsPSO> GetPSO() { return m_pso; }
	std::shared_ptr<RootSignature> GetRootSignature() { return m_rootSig; }

	void Finalize() override;

	struct Signature
	{
		// Per-view and per-object CBV bindings
		uint32_t	perViewDataIndex{ kInvalid };
		uint32_t	perObjectDataIndex{ kInvalid };
		uint32_t	perViewDataSize{ 0 };   // For validation
		uint32_t	perObjectDataSize{ 0 }; // For validation

		// Count of total CPU descriptors in the master array
		uint32_t	totalDescriptors{ 0 };

		// Size in bytes of the CPU backing store for the per-material CBVs
		uint32_t	cbvPerMaterialDataSize{ kInvalid };

		// Root parameters
		std::vector<ShaderReflection::DescriptorRange> rootParameters;

		// Mapping data
		std::map<std::string, ShaderReflection::Parameter<5>>		parameters;
		std::array<std::vector<ShaderReflection::CBVLayout>, 5>		cbvBindings;
		std::map<std::string, ShaderReflection::ResourceSRV<5>>		srvs;
		std::map<std::string, ShaderReflection::ResourceUAV<5>>		uavs;
		// TODO samplers
	};

private:
	void BuildEffectSignature();
	void BuildPSO();

	void CreateRootSignature();
	void ProcessShaderBindings(uint32_t& rootIndex, Shader* shader);
	D3D12_ROOT_SIGNATURE_FLAGS GetRootSignatureFlags();

private:
	std::shared_ptr<RootSignature>	m_rootSig;
	std::shared_ptr<GraphicsPSO>	m_pso;

	Signature						m_signature;
};

} // namespace Kodiak