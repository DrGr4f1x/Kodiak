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

} // namespace Kodiak