// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

// Forward declarations
namespace Kodiak
{
enum class ShaderResourceDimension;
enum class ShaderResourceType;
enum class ShaderVariableType;
}

namespace ShaderReflection
{

struct CBVLayout
{
	std::string		name;
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


struct DescriptorRange
{
	DescriptorRange() {}
	explicit DescriptorRange(uint32_t _startSlot) : startSlot(_startSlot) {}
	DescriptorRange(uint32_t _startSlot, uint32_t _numElements) : startSlot(_startSlot), numElements(_numElements) {}

	uint32_t		startSlot{ kInvalid };
	uint32_t		numElements{ kInvalid };
};


struct BaseParameter
{
	BaseParameter() = default;
	BaseParameter(const BaseParameter& other) : name(other.name), type(other.type), sizeInBytes(other.sizeInBytes) {}

	std::string							name;
	Kodiak::ShaderVariableType			type;
	uint32_t							sizeInBytes{ 0 };
};


template <uint32_t SlotCount = 1>
struct Parameter : public BaseParameter
{
	Parameter()
	{
		cbvShaderRegister.fill(kInvalid);
		byteOffset.fill(kInvalid);
	}

	Parameter(uint32_t slot, const Parameter<1>& other) : BaseParameter(other)
	{
		cbvShaderRegister.fill(kInvalid);
		byteOffset.fill(kInvalid);

		assert(slot < SlotCount);
		cbvShaderRegister[slot] = other.cbvShaderRegister[0];
		byteOffset[slot] = other.byteOffset[0];
	}

	void Assign(uint32_t slot, const Parameter<1>& other)
	{
		assert(slot < SlotCount);
		assert(type == other.type);
		assert(sizeInBytes == other.sizeInBytes);
		cbvShaderRegister[slot] = other.cbvShaderRegister[0];
		byteOffset[slot] = other.byteOffset[0];
	}

	std::array<uint32_t, SlotCount>		cbvShaderRegister;
	std::array<uint32_t, SlotCount>		byteOffset;
};


struct BaseResourceSRV
{
	BaseResourceSRV() = default;
	BaseResourceSRV(const BaseResourceSRV& other) : name(other.name), type(other.type), dimension(other.dimension) {}

	std::string							name;
	Kodiak::ShaderResourceType			type;
	Kodiak::ShaderResourceDimension		dimension;
};


template <uint32_t SlotCount = 1>
struct ResourceSRV : public BaseResourceSRV
{
	ResourceSRV()
	{
		shaderRegister.fill(kInvalid);
	}

	ResourceSRV(uint32_t slot, const ResourceSRV<1>& other) : BaseResourceSRV(other)
	{
		shaderRegister.fill(kInvalid);

		assert(slot < SlotCount);
		binding[slot] = other.binding[0];
		shaderRegister[slot] = other.shaderRegister[0];
	}

	void Assign(uint32_t slot, const ResourceSRV<1>& other)
	{
		assert(slot < SlotCount);
		assert(type == other.type);
		assert(dimension == other.dimension);
		binding[slot] = other.binding[0];
		shaderRegister[slot] = other.shaderRegister[0];
	}

	std::array<TableEntry, SlotCount>	binding;
	std::array<uint32_t, SlotCount>		shaderRegister;
};


struct BaseResourceUAV
{
	BaseResourceUAV() = default;
	BaseResourceUAV(const BaseResourceUAV& other) : name(other.name), type(other.type) {}

	std::string							name;
	Kodiak::ShaderResourceType			type;
};


template <uint32_t SlotCount = 1>
struct ResourceUAV : public BaseResourceUAV
{
	ResourceUAV()
	{
		shaderRegister.fill(kInvalid);
	}

	ResourceUAV(uint32_t slot, const ResourceUAV<1>& other) : BaseResourceUAV(other)
	{
		shaderRegister.fill(kInvalid);

		assert(slot < SlotCount);
		binding[slot] = other.binding[0];
		shaderRegister[slot] = other.shaderRegister[0];
	}

	void Assign(uint32_t slot, const ResourceUAV<1>& other)
	{
		assert(slot < SlotCount);
		assert(type == other.type);
		binding[slot] = other.binding[0];
		shaderRegister[slot] = other.shaderRegister[0];
	}

	std::array<TableEntry, SlotCount>	binding;
	std::array<uint32_t, SlotCount>		shaderRegister;
};


struct BaseSampler
{
	BaseSampler() = default;
	BaseSampler(const BaseSampler& other) : name(other.name) {}
	std::string							name;
};

template <uint32_t SlotCount = 1>
struct Sampler : public BaseSampler
{
	Sampler()
	{
		shaderRegister.fill(kInvalid);
	}

	Sampler(uint32_t slot, const Sampler<1>& other) : BaseSampler(other)
	{
		shaderRegister.fill(kInvalid);

		assert(slot < SlotCount);
		binding[slot] = other.binding[0];
		shaderRegister[slot] = other.shaderRegister[0];
	}

	void Assign(uint32_t slot, const Sampler<1>& other)
	{
		assert(slot < SlotCount);
		binding[slot] = other.binding[0];
		shaderRegister[slot] = other.shaderRegister[0];
	}

	std::array<TableEntry, SlotCount>	binding;
	std::array<uint32_t, SlotCount>		shaderRegister;
};


// Signature of a single shader
struct Signature
{
	// DX API inputs
	ShaderReflection::CBVLayout						cbvPerViewData;
	ShaderReflection::CBVLayout						cbvPerObjectData;
	std::vector<ShaderReflection::CBVLayout>		cbvTable;
	std::vector<ShaderReflection::TableLayout>		srvTable;
	std::vector<ShaderReflection::TableLayout>		uavTable;
	std::vector<ShaderReflection::TableLayout>		samplerTable;

	// Application inputs
	std::vector<ShaderReflection::Parameter<1>>		parameters;
	std::vector<ShaderReflection::ResourceSRV<1>>	resources;
	std::vector<ShaderReflection::ResourceUAV<1>>	uavs;
	std::vector<ShaderReflection::Sampler<1>>		samplers;

	// Additional data
	uint32_t										numDescriptors{ 0 };
	uint32_t										numMaterialDescriptors{ 0 };
	uint32_t										numSamplers{ 0 };
};


//
// Utility functions
//
Kodiak::ShaderVariableType ConvertToEngine(D3D_SHADER_VARIABLE_TYPE type, uint32_t rows, uint32_t columns);
Kodiak::ShaderResourceDimension ConvertToEngine(D3D_SRV_DIMENSION dim);

#if defined(DX11)
using ID3DShaderReflection			= ID3D11ShaderReflection;
using D3D_SHADER_INPUT_BIND_DESC	= D3D11_SHADER_INPUT_BIND_DESC;
using D3D_SHADER_BUFFER_DESC		= D3D11_SHADER_BUFFER_DESC;
using D3D_SHADER_VARIABLE_DESC		= D3D11_SHADER_VARIABLE_DESC;
using D3D_SHADER_TYPE_DESC			= D3D11_SHADER_TYPE_DESC;
using D3D_SHADER_TYPE_DESC			= D3D11_SHADER_TYPE_DESC;
using D3D_SHADER_DESC				= D3D11_SHADER_DESC;
#elif defined(DX12)
using ID3DShaderReflection			= ID3D12ShaderReflection;
using D3D_SHADER_INPUT_BIND_DESC	= D3D12_SHADER_INPUT_BIND_DESC;
using D3D_SHADER_BUFFER_DESC		= D3D12_SHADER_BUFFER_DESC;
using D3D_SHADER_VARIABLE_DESC		= D3D12_SHADER_VARIABLE_DESC;
using D3D_SHADER_TYPE_DESC			= D3D12_SHADER_TYPE_DESC;
using D3D_SHADER_DESC				= D3D12_SHADER_DESC;
#else
#error Not using DirectX!
#endif

void IntrospectCBuffer(ID3DShaderReflection* reflector, const D3D_SHADER_INPUT_BIND_DESC& inputDesc, Signature& signature);
void IntrospectResourceSRV(Kodiak::ShaderResourceType type, const D3D_SHADER_INPUT_BIND_DESC& inputDesc, Signature& signature);
void IntrospectResourceUAV(Kodiak::ShaderResourceType type, const D3D_SHADER_INPUT_BIND_DESC& inputDesc, Signature& signature);
void IntrospectSampler(const D3D_SHADER_INPUT_BIND_DESC& inputDesc, Signature& signature);
void ResetSignature(Signature& signature);

void Introspect(ID3DShaderReflection* reflector, Signature& signature);

} // namespace ShaderReflection