// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

template<class ShaderClass>
void ShaderManager::LoadShaderAsync(std::shared_ptr<ShaderClass> shader, const std::string& shaderPath) const
{
	// Set initial state on the shader
	shader->m_shader = nullptr;
	shader->m_isReady = false;

	using namespace concurrency;
	shader->loadTask = create_task([shader, shaderPath]()
	{
		// Load the compiled shader file
		unique_ptr<uint8_t[]> data;
		size_t dataSize;
		
		// Try common DX shader first, i.e. shader.dx.cso
		HRESULT res = S_OK;

		std::string commonPath = shaderPath + ".dx.cso";
		res = BinaryReader::ReadEntireFile(commonPath, data, &dataSize);
		if (res != S_OK)
		{
			// Try dx11 shader, i.e. shader.dx11.cso
			std::string dx11Path = shaderPath + ".dx11.cso";
			ThrowIfFailed(BinaryReader::ReadEntireFile(dx11Path, data, &dataSize));
		}

		// Create the shader
		shader->Create(data, dataSize);

		// Finalize shader state
		shader->m_isReady = true;
	});
}


template<class ShaderClass>
void ShaderManager::LoadShaderSerial(std::shared_ptr<ShaderClass> shader, const std::string& shaderPath) const
{
	// Set initial state on the shader
	shader->m_shader = nullptr;
	shader->m_isReady = false;
	shader->loadTask = concurrency::create_task([] {});

	// Load the compiled shader file
	unique_ptr<uint8_t[]> data;
	size_t dataSize;
	
	// Try common DX shader first, i.e. shader.dx.cso
	HRESULT res = S_OK;

	std::string commonPath = shaderPath + ".dx.cso";
	res = BinaryReader::ReadEntireFile(commonPath, data, &dataSize);
	if (res != S_OK)
	{
		// Try dx11 shader, i.e. shader.dx11.cso
		std::string dx11Path = shaderPath + ".dx11.cso";
		ThrowIfFailed(BinaryReader::ReadEntireFile(dx11Path, data, &dataSize));
	}

	// Create the shader
	shader->Create(data, dataSize);

	// Finalize shader state
	shader->m_isReady = true;
}