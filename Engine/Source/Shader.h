// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#if defined(DX12)
#include "Shader12.h"
#elif defined(DX11)
#include "Shader11.h"
#elif defined(VK)
#include "ShaderVk.h"
#else
#error No graphics API defined!
#endif


namespace Kodiak
{

class ShaderPath
{
public:
	ShaderPath() {}
	explicit ShaderPath(const std::string& shaderFile);
	ShaderPath(const std::string& shaderPath, const std::string& shaderFile);

	const std::string& GetFullPath() const { return m_shaderFullPath; }
	bool HasPath() const { return !m_shaderFullPath.empty(); }

private:
	std::string m_shaderPath;
	std::string m_shaderFile;
	std::string m_shaderFullPath;
};


struct ShaderConstantBufferDesc
{
	ShaderConstantBufferDesc(const char* _name, uint32_t _registerSlot, uint32_t _size)
		: name(_name), registerSlot(_registerSlot), size(_size), variables()
	{}

	ShaderConstantBufferDesc(ShaderConstantBufferDesc&& other)
		: name(std::move(other.name)), registerSlot(other.registerSlot), size(other.size), variables(std::move(other.variables))
	{}

	ShaderConstantBufferDesc& operator=(const ShaderConstantBufferDesc& other) = default;

	std::string name;
	uint32_t registerSlot;
	uint32_t size;
	std::vector<ShaderVariableDesc> variables;
};


struct ShaderResourceDesc
{
	ShaderResourceDesc(const char* _name, uint32_t _slot, ShaderResourceDimension _dimension)
		: name(_name), slot(_slot), dimension(_dimension)
	{}

	ShaderResourceDesc(ShaderResourceDesc&& other)
		: name(std::move(other.name)), slot(other.slot), dimension(other.dimension)
	{}

	ShaderResourceDesc& operator=(const ShaderResourceDesc& other) = default;

	std::string name;
	uint32_t slot;
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


} // namespace Kodiak