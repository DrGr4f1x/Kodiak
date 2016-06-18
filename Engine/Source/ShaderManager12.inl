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
	shader->m_byteCode.reset();
	shader->m_isReady = false;

	using namespace concurrency;
	shader->loadTask = create_task([shader, shaderPath]()
	{
		// Try common DX shader first, i.e. shader.dx.cso
		HRESULT res = S_OK;

		std::string commonPath = shaderPath + ".dx.cso";
		res = BinaryReader::ReadEntireFile(commonPath, shader->m_byteCode, &shader->m_byteCodeSize);
		if (res != S_OK)
		{
			// Try dx12 shader, i.e. shader.dx12.cso
			std::string dx12Path = shaderPath + ".dx12.cso";
			ThrowIfFailed(BinaryReader::ReadEntireFile(dx12Path, shader->m_byteCode, &shader->m_byteCodeSize));
		}
		
		// Introspect
		shader->Finalize();

		// Finalize shader state
		shader->m_isReady = true;
	});
}


template<class ShaderClass>
void ShaderManager::LoadShaderSerial(std::shared_ptr<ShaderClass> shader, const std::string& shaderPath) const
{
	// Set initial state on the shader
	shader->m_byteCode.reset();
	shader->m_isReady = false;

	using namespace concurrency;
	shader->loadTask = create_task([] {});

	// Try common DX shader first, i.e. shader.dx.cso
	HRESULT res = S_OK;

	std::string commonPath = shaderPath + ".dx.cso";
	res = BinaryReader::ReadEntireFile(commonPath, shader->m_byteCode, &shader->m_byteCodeSize);
	if (res != S_OK)
	{
		// Try dx12 shader, i.e. shader.dx12.cso
		std::string dx12Path = shaderPath + ".dx12.cso";
		ThrowIfFailed(BinaryReader::ReadEntireFile(dx12Path, shader->m_byteCode, &shader->m_byteCodeSize));
	}

	// Introspect
	shader->Finalize();

	// Finalize shader state
	shader->m_isReady = true;
}