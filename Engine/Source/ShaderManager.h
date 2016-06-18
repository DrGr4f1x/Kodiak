// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "BinaryReader.h"
#include "RenderUtils.h"

namespace Kodiak
{

// Forward declarations
class ComputeShader;
class DomainShader;
class GeometryShader;
class HullShader;
class ShaderPath;
class PixelShader;
class VertexShader;


class ShaderManager
{
public:
	static ShaderManager& GetInstance();

	static void DestroyAll();

	std::shared_ptr<ComputeShader> LoadComputeShader(const std::string& shaderPath, bool asyncLoad = true) const;
	std::shared_ptr<DomainShader> LoadDomainShader(const std::string& shaderPath, bool asyncLoad = true) const;
	std::shared_ptr<GeometryShader> LoadGeometryShader(const std::string& shaderPath, bool asyncLoad = true) const;
	std::shared_ptr<HullShader>	LoadHullShader(const std::string& shaderPath, bool asyncLoad = true) const;
	std::shared_ptr<PixelShader> LoadPixelShader(const std::string& shaderPath, bool asyncLoad = true) const;
	std::shared_ptr<VertexShader> LoadVertexShader(const std::string& shaderPath, bool asyncLoad = true) const;

private:
	template<class ShaderClass>
	void LoadShaderAsync(std::shared_ptr<ShaderClass>shader, const std::string& fullPathToShader) const;

	template<class ShaderClass>
	void LoadShaderSerial(std::shared_ptr<ShaderClass> shader, const std::string& fullPathToShader) const;
};


#if defined(DX12)
#include "ShaderManager12.inl"
#elif defined(DX11)
#include "ShaderManager11.inl"
#elif defined(VK)
#include "ShaderManagerVk.inl"
#else
#error No graphics API defined!
#endif

} // namespace Kodiak