// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "MaterialManager11.h"

#include "Material11.h"
#include "RenderUtils.h"
#include "ShaderManager11.h"

#include <map>

using namespace Kodiak;
using namespace std;


namespace
{ 

std::map<size_t, std::shared_ptr<Material>>		s_materialHashMap;
std::map<size_t, MaterialDesc>					s_materialDescHashMap;

} // anonymous namespace


MaterialManager& MaterialManager::GetInstance()
{
	static MaterialManager instance;
	return instance;
}


void MaterialManager::Destroy()
{
	s_materialHashMap.clear();
	s_materialDescHashMap.clear();
}


shared_ptr<Material> MaterialManager::CreateMaterial(const MaterialDesc& desc, bool asyncCreate)
{
	auto hashCode = ComputeHash(desc);

	shared_ptr<Material> material;

	{
		static mutex materialMutex;
		lock_guard<mutex> CS(materialMutex);

		auto iter = s_materialHashMap.find(hashCode);

		if (iter == s_materialHashMap.end())
		{
			material = make_shared<Material>();
			s_materialHashMap[hashCode] = material;

			if (asyncCreate)
			{
				// Non-blocking asynchronous material creation
				CreateMaterialAsync(material, desc);
			}
			else
			{
				// Blocking synchronous material creation
				CreateMaterialSerial(material, desc);
			}
		}
		else
		{
			material = iter->second;
		}
	}

	return material;
}


void MaterialManager::CreateMaterialAsync(shared_ptr<Material> material, const MaterialDesc& desc)
{
	// Verify we have a vertex shader (required)
	assert(!desc.vertexShaderFile.empty());

	auto& shaderManager = ShaderManager::GetInstance();

	// Load vertex shader
	auto vs = shaderManager.LoadVertexShader(desc.vertexShaderPath, desc.vertexShaderFile, true);
	material->loadTask = vs->loadTask;
	material->m_vertexShader = vs;

	// Load domain shader (optional)
	if (!desc.domainShaderFile.empty())
	{
		auto ds = shaderManager.LoadDomainShader(desc.domainShaderPath, desc.domainShaderFile, true);
		material->loadTask = (material->loadTask && ds->loadTask);
		material->m_domainShader = ds;
	}

	// Load hull shader (optional)
	if (!desc.hullShaderFile.empty())
	{
		auto hs = shaderManager.LoadHullShader(desc.hullShaderPath, desc.hullShaderFile, true);
		material->loadTask = (material->loadTask && hs->loadTask);
		material->m_hullShader = hs;
	}

	// Load geometry shader (optional)
	if (!desc.geometryShaderFile.empty())
	{
		auto gs = shaderManager.LoadGeometryShader(desc.geometryShaderPath, desc.geometryShaderFile, true);
		material->loadTask = (material->loadTask && gs->loadTask);
		material->m_geometryShader = gs;
	}

	// Load pixel shader (optional)
	if (!desc.pixelShaderFile.empty())
	{
		auto ps = shaderManager.LoadPixelShader(desc.pixelShaderPath, desc.pixelShaderFile, true);
		material->loadTask = (material->loadTask && ps->loadTask);
		material->m_pixelShader = ps;
	}

	// Configure the graphics PSO
}


void MaterialManager::CreateMaterialSerial(shared_ptr<Material> material, const MaterialDesc& desc)
{

}