// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "Shader11.h"

namespace Kodiak
{

// Forward declarations
class ComputeShader;
class DomainShader;
class GeometryShader;
class HullShader;
class PixelShader;
class VertexShader;


class ShaderManager
{
public:
	static ShaderManager& GetInstance();

	static void DestroyAll();

	std::shared_ptr<ComputeShader> LoadComputeShader(const std::string& shaderPath, const std::string& shaderFile, bool asyncLoad = true) const;
	std::shared_ptr<DomainShader> LoadDomainShader(const std::string& shaderPath, const std::string& shaderFile, bool asyncLoad = true) const;
	std::shared_ptr<GeometryShader> LoadGeometryShader(const std::string& shaderPath, const std::string& shaderFile, bool asyncLoad = true) const;
	std::shared_ptr<HullShader> LoadHullShader(const std::string& shaderPath, const std::string& shaderFile, bool asyncLoad = true) const;
	std::shared_ptr<PixelShader> LoadPixelShader(const std::string& shaderPath, const std::string& shaderFile, bool asyncLoad = true) const;
	std::shared_ptr<VertexShader> LoadVertexShader(const std::string& shaderPath, const std::string& shaderFile, bool asyncLoad = true) const;

private:
	template<class ShaderClass>
	void LoadShaderAsync(std::shared_ptr<ShaderClass>shader, const std::string& fullPathToShader) const;

	template<class ShaderClass>
	void LoadShaderSerial(std::shared_ptr<ShaderClass> shader, const std::string& fullPathToShader) const;
};


template<class ShaderClass>
void ShaderManager::LoadShaderAsync(std::shared_ptr<ShaderClass> shader, const std::string& fullPathToShader) const
{
	// Set initial state on the shader
	shader->m_shader = nullptr;
	shader->m_isReady = false;

	using namespace concurrency;
	shader->loadTask = create_task([shader, fullPathToShader]()
	{
		// Load the compiled shader file
		unique_ptr<uint8_t[]> data;
		size_t dataSize;
		ThrowIfFailed(BinaryReader::ReadEntireFile(fullPathToShader, data, &dataSize));

		// Create the shader
		shader->Create(data, dataSize);

		// Finalize shader state
		shader->m_isReady = true;
	});
}


template<class ShaderClass>
void ShaderManager::LoadShaderSerial(std::shared_ptr<ShaderClass> shader, const std::string& fullPathToShader) const
{
	// Set initial state on the shader
	shader->m_shader = nullptr;
	shader->m_isReady = false;

	// Load the compiled shader file
	unique_ptr<uint8_t[]> data;
	size_t dataSize;
	ThrowIfFailed(BinaryReader::ReadEntireFile(fullPathToShader, data, &dataSize));

	// Create the shader
	shader->Create(data, dataSize);

	// Finalize shader state
	shader->m_isReady = true;
}


} // namespace Kodiak