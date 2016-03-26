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


template <uint32_t SlotCount = 1>
struct Parameter
{
	Parameter()
	{
		cbvShaderRegister.fill(kInvalid);
		byteOffset.fill(kInvalid);
	}
	std::string							name;
	Kodiak::ShaderVariableType			type;
	uint32_t							sizeInBytes{ 0 }; 
	std::array<uint32_t, SlotCount>		cbvShaderRegister;
	std::array<uint32_t, SlotCount>		byteOffset;
};


template <uint32_t SlotCount = 1>
struct ResourceSRV
{
	std::string							name;
	Kodiak::ShaderResourceType			type;
	Kodiak::ShaderResourceDimension		dimension;
	std::array<TableEntry, SlotCount>	binding;
};


template <uint32_t SlotCount = 1>
struct ResourceUAV
{
	std::string							name;
	Kodiak::ShaderResourceType			type;
	std::array<TableEntry, SlotCount>	binding;
};


template <uint32_t SlotCount = 1>
struct Sampler
{
	std::string							name;
	std::array<TableEntry, SlotCount>	binding;
};


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