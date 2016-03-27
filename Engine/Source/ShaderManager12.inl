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
void ShaderManager::LoadShaderAsync(std::shared_ptr<ShaderClass> shader, const std::string& fullPathToShader) const
{
	// Set initial state on the shader
	shader->m_byteCode.reset();
	shader->m_isReady = false;

	using namespace concurrency;
	shader->loadTask = create_task([shader, fullPathToShader]()
	{
		// Load the compiled shader file
		ThrowIfFailed(BinaryReader::ReadEntireFile(fullPathToShader, shader->m_byteCode, &shader->m_byteCodeSize));

		// Introspect
		shader->Finalize();

		// Finalize shader state
		shader->m_isReady = true;
	});
}


template<class ShaderClass>
void ShaderManager::LoadShaderSerial(std::shared_ptr<ShaderClass> shader, const std::string& fullPathToShader) const
{
	// Set initial state on the shader
	shader->m_byteCode.reset();
	shader->m_isReady = false;

	using namespace concurrency;
	shader->loadTask = create_task([] {});

	// Load the compiled shader file
	ThrowIfFailed(BinaryReader::ReadEntireFile(fullPathToShader, shader->m_byteCode, &shader->m_byteCodeSize));

	// Introspect
	shader->Finalize();

	// Finalize shader state
	shader->m_isReady = true;
}