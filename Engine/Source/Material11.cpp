// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Material11.h"

#include "InputLayout11.h"
#include "Paths.h"
#include "RenderUtils.h"
#include "Shader11.h"
#include "ShaderManager11.h"


using namespace Kodiak;
using namespace std;


namespace Kodiak
{

size_t ComputeHash(const MaterialDesc& desc)
{
	size_t hashVal = 0;

	auto& shaderRootPath = Paths::GetInstance().ShaderDir();

	// Vertex shader
	{
		auto fullPath = shaderRootPath + desc.vertexShaderPath + "\\SM5\\" + desc.vertexShaderFile;

		hash<string> hashFunc;
		size_t hashCode = hashFunc(fullPath);

		hashVal = HashIterate(hashCode);
	}

	// Domain shader
	if (!desc.domainShaderPath.empty() || !desc.domainShaderFile.empty())
	{
		auto fullPath = shaderRootPath + desc.domainShaderPath + "\\SM5\\" + desc.domainShaderFile;

		hash<string> hashFunc;
		size_t hashCode = hashFunc(fullPath);

		hashVal = HashIterate(hashCode, hashVal);
	}

	// Hull shader
	if (!desc.hullShaderPath.empty() || !desc.hullShaderFile.empty())
	{
		auto fullPath = shaderRootPath + desc.hullShaderPath + "\\SM5\\" + desc.hullShaderFile;

		hash<string> hashFunc;
		size_t hashCode = hashFunc(fullPath);

		hashVal = HashIterate(hashCode, hashVal);
	}

	// Geometry shader
	if (!desc.geometryShaderPath.empty() || !desc.geometryShaderFile.empty())
	{
		auto fullPath = shaderRootPath + desc.geometryShaderPath + "\\SM5\\" + desc.geometryShaderFile;

		hash<string> hashFunc;
		size_t hashCode = hashFunc(fullPath);

		hashVal = HashIterate(hashCode, hashVal);
	}

	// Pixel shader
	if (!desc.pixelShaderPath.empty() || !desc.pixelShaderFile.empty())
	{
		auto fullPath = shaderRootPath + desc.pixelShaderPath + "\\SM5\\" + desc.pixelShaderFile;

		hash<string> hashFunc;
		size_t hashCode = hashFunc(fullPath);

		hashVal = HashIterate(hashCode, hashVal);
	}

	// State
	hashVal = HashState(&desc.blendStateDesc, hashVal);
	hashVal = HashState(&desc.depthStencilStateDesc, hashVal);
	hashVal = HashState(&desc.rasterizerStateDesc);
}

} // namespace Kodiak


Material::Material(const MaterialDesc& desc)
{
	Create(desc);
}


void Material::Create(const MaterialDesc& desc)
{
}


void Material::SetVertexShaderPath(const string& shaderPath, const string& shaderFile)
{
	auto vs = ShaderManager::GetInstance().LoadVertexShader(shaderPath, shaderFile);
	SetVertexShader(vs);
}


void Material::SetVertexShader(shared_ptr<VertexShader> vertexShader)
{
	m_isFinalized = false;
	m_vertexShader = vertexShader;

	if (m_isTaskValid)
	{
		loadTask = loadTask && m_vertexShader->loadTask;
	}
	else
	{
		loadTask = m_vertexShader->loadTask;
		m_isTaskValid = true;
	}
}


void Material::SetDomainShaderPath(const string& shaderPath, const string& shaderFile)
{
	auto ds = ShaderManager::GetInstance().LoadDomainShader(shaderPath, shaderFile);
	SetDomainShader(ds);
}


void Material::SetDomainShader(shared_ptr<DomainShader> domainShader)
{
	m_isFinalized = false;
	m_domainShader = domainShader;

	if (m_isTaskValid)
	{
		loadTask = loadTask && m_domainShader->loadTask;
	}
	else
	{
		loadTask = m_domainShader->loadTask;
		m_isTaskValid = true;
	}
}


void Material::SetHullShaderPath(const string& shaderPath, const string& shaderFile)
{
	auto hs = ShaderManager::GetInstance().LoadHullShader(shaderPath, shaderFile);
	SetHullShader(hs);
}


void Material::SetHullShader(shared_ptr<HullShader> hullShader)
{
	m_isFinalized = false;
	m_hullShader = hullShader;

	if (m_isTaskValid)
	{
		loadTask = loadTask && m_hullShader->loadTask;
	}
	else
	{
		loadTask = m_hullShader->loadTask;
		m_isTaskValid = true;
	}
}