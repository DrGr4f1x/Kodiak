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

	struct CBVBinding
	{
		uint32_t		byteOffset{ kInvalid };		// offset from start of large cbuffer (16-byte aligned)
		uint32_t		sizeInBytes{ kInvalid };    // cbuffer size in bytes (multiple of 16)
		uint32_t		shaderRegister{ kInvalid };
	};

	struct TableLayout
	{
		uint32_t		shaderRegister{ kInvalid };
		uint32_t		numItems{ kInvalid };
	};

	struct TableEntry
	{
		uint32_t		tableIndex{ kInvalid };
		uint32_t		tableSlot{ kInvalid };
	};

	struct Parameter
	{
		std::string					name;
		ShaderVariableType			type;
		uint32_t					sizeInBytes{ 0 };
		std::array<uint32_t, 5>		byteOffsets{ kInvalid, kInvalid, kInvalid, kInvalid, kInvalid };
		std::array<uint32_t, 5>		cbvShaderRegister{ kInvalid, kInvalid, kInvalid, kInvalid, kInvalid };
	};

	struct ResourceSRV
	{
		std::string					name;
		ShaderResourceType			type;
		ShaderResourceDimension		dimension;
		std::array<TableEntry, 5>	bindings;
	};

	struct ResourceUAV
	{
		std::string					name;
		ShaderResourceType			type;
		std::array<TableEntry, 5>	bindings;
	};

	struct Sampler
	{
		std::string					name;
		std::array<TableEntry, 5>	bindings;
	};

	struct Signature
	{
		// Per-view and per-object CBV bindings
		std::array<uint32_t, 5> perViewDataBindings;
		std::array<uint32_t, 5> perObjectDataBindings;
		uint32_t				perViewDataSize;	// For validation
		uint32_t				perObjectDataSize;	// For validation

		// Per-material CBV bindings (one large cbuffer for everything)
		uint32_t cbvPerMaterialDataSize{ kInvalid };
		std::array<std::vector<CBVBinding>, 5> cbvBindings;

		// Resource bindings
		std::array<std::vector<TableLayout>, 5> srvBindings;
		std::array<std::vector<TableLayout>, 5> uavBindings;
		std::array<std::vector<TableLayout>, 5> samplerBindings;

		// Parameters and resources
		std::vector<Parameter>					parameters;
		std::vector<ResourceSRV>				srvs;
		std::vector<ResourceUAV>				uavs;
		std::vector<Sampler>					samplers;
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
