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
#include "PipelineState11.h"
#include "Shader11.h"
#include "ShaderManager11.h"


using namespace Kodiak;
using namespace std;


Material::Material()
	: m_pso(make_shared<GraphicsPSO>())
	, m_name()
{}


Material::Material(const string& name)
	: m_pso(make_shared<GraphicsPSO>())
	, m_name(name)
{}


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