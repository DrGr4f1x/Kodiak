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

} // namespace Kodiak