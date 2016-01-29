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

struct ShaderConstantBufferDesc
{
	ShaderConstantBufferDesc(const char* _name, uint32_t _registerSlot, uint32_t _size)
		: name(_name), registerSlot(_registerSlot), size(_size)
	{}

	ShaderConstantBufferDesc(ShaderConstantBufferDesc&& other)
		: name(std::move(other.name)), registerSlot(other.registerSlot), size(other.size)
	{}

	ShaderConstantBufferDesc& operator=(const ShaderConstantBufferDesc& other) = default;

	std::string		name;
	uint32_t		registerSlot;
	uint32_t		size;
};


struct ShaderResourceDesc
{
	ShaderResourceDesc(const char* _name, uint32_t _registerSlot, ShaderResourceType _type, ShaderResourceDimension _dimension)
		: name(_name), registerSlot(_registerSlot), type(_type), dimension(_dimension)
	{}

	ShaderResourceDesc(ShaderResourceDesc&& other)
		: name(std::move(other.name)), registerSlot(other.registerSlot), type(other.type), dimension(other.dimension)
	{}

	ShaderResourceDesc& operator=(const ShaderResourceDesc& other) = default;

	std::string				name;
	uint32_t				registerSlot;
	ShaderResourceType		type;
	ShaderResourceDimension dimension;
};


struct ShaderVariableDesc
{
	ShaderVariableDesc(const char* _name, uint32_t _constantBuffer, uint32_t _startOffset, uint32_t _size, ShaderVariableType _type)
		: name(_name), constantBuffer(_constantBuffer), startOffset(_startOffset), size(_size), type(_type)
	{}

	ShaderVariableDesc(ShaderVariableDesc&& other)
		: name(std::move(other.name)), constantBuffer(other.constantBuffer), startOffset(other.startOffset), size(other.size), type(other.type)
	{}

	ShaderVariableDesc& operator=(const ShaderVariableDesc& other) = default;

	std::string name;
	uint32_t constantBuffer;
	uint32_t startOffset;
	uint32_t size;
	ShaderVariableType type;
};


struct ShaderBindingDesc
{
	std::vector<ShaderConstantBufferDesc>	cbuffers;
	std::vector<ShaderResourceDesc>			resources;

	size_t									perViewDataSize{ 0 };
	size_t									perObjectDataSize{ 0 };
};

} // namespace Kodiak