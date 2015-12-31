// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "MaterialManager.h"

#include "Material.h"
#include "RenderUtils.h"
#include "ShaderManager.h"

#include <map>
#include <ppltasks.h>


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


shared_ptr<Material> MaterialManager::CreateMaterial(const string& name, shared_ptr<MaterialDesc> desc, bool asyncCreate)
{
	auto hashCode = ComputeHash(*desc);

	shared_ptr<Material> material;

	{
		static mutex materialMutex;
		lock_guard<mutex> CS(materialMutex);

		auto iter = s_materialHashMap.find(hashCode);

		if (iter == s_materialHashMap.end())
		{
			material = shared_ptr<Material>(new Material(name));
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


void MaterialManager::CreateMaterialAsync(shared_ptr<Material> material, std::shared_ptr<MaterialDesc> desc)
{
	using namespace concurrency;

	// Verify we have a vertex shader (required)
	assert(desc->vertexShaderPath.HasPath());

	create_task([material, desc]
	{
		auto& shaderManager = ShaderManager::GetInstance();

		ShaderState shaderState;

		// Create PSO for material
		material->m_pso = make_shared<GraphicsPSO>();

		// Load vertex shader
		auto vs = shaderManager.LoadVertexShader(desc->vertexShaderPath, true);
		shaderState.vertexShader = vs;

		// Once the shader has finished loading, set it on the PSO
		auto psoTask = vs->loadTask.then([material, vs]
		{
			material->m_pso->SetVertexShader(vs.get());
			material->m_pso->SetInputLayout(*vs->GetInputLayout());
		});

		// Load domain shader (optional)
		if (desc->domainShaderPath.HasPath())
		{
			auto ds = shaderManager.LoadDomainShader(desc->domainShaderPath, true);
			shaderState.domainShader = ds;

			// Once the shader has finished loading, set it on the PSO
			psoTask = psoTask && ds->loadTask.then([material, ds]
			{
				material->m_pso->SetDomainShader(ds.get());
			});
		}

		// Load hull shader (optional)
		if (desc->hullShaderPath.HasPath())
		{
			auto hs = shaderManager.LoadHullShader(desc->hullShaderPath, true);
			shaderState.hullShader = hs;

			// Once the shader has finished loading, set it on the PSO
			psoTask = psoTask && hs->loadTask.then([material, hs]
			{
				material->m_pso->SetHullShader(hs.get());
			});
		}

		// Load geometry shader (optional)
		if (desc->geometryShaderPath.HasPath())
		{
			auto gs = shaderManager.LoadGeometryShader(desc->geometryShaderPath, true);
			shaderState.geometryShader = gs;

			// Once the shader has finished loading, set it on the PSO
			psoTask = psoTask && gs->loadTask.then([material, gs]
			{
				material->m_pso->SetGeometryShader(gs.get());
			});
		}

		// Load pixel shader (optional)
		if (desc->pixelShaderPath.HasPath())
		{
			auto ps = shaderManager.LoadPixelShader(desc->pixelShaderPath, true);
			shaderState.pixelShader = ps;

			// Once the shader has finished loading, set it on the PSO
			psoTask = psoTask && ps->loadTask.then([material, ps]
			{
				material->m_pso->SetPixelShader(ps.get());
			});
		}

		// Finalize the graphics PSO & bind material parameters
		material->SetupPSO(*desc);
		psoTask.wait();
		material->m_pso->Finalize();

		material->BindParameters(shaderState);

		material->m_isReady = true;
	});
}


void MaterialManager::CreateMaterialSerial(shared_ptr<Material> material, std::shared_ptr<MaterialDesc> desc)
{
	// Verify we have a vertex shader (required)
	assert(desc->vertexShaderPath.HasPath());

	auto& shaderManager = ShaderManager::GetInstance();

	ShaderState shaderState;

	// Create PSO for material
	material->m_pso = make_shared<GraphicsPSO>();

	// Load vertex shader
	auto vs = shaderManager.LoadVertexShader(desc->vertexShaderPath, false);
	shaderState.vertexShader = vs;

	material->m_pso->SetVertexShader(vs.get());
	material->m_pso->SetInputLayout(*vs->GetInputLayout());
	
	// Load domain shader (optional)
	if (desc->domainShaderPath.HasPath())
	{
		auto ds = shaderManager.LoadDomainShader(desc->domainShaderPath, false);
		shaderState.domainShader = ds;

		material->m_pso->SetDomainShader(ds.get());
	}

	// Load hull shader (optional)
	if (desc->hullShaderPath.HasPath())
	{
		auto hs = shaderManager.LoadHullShader(desc->hullShaderPath, false);
		shaderState.hullShader = hs;

		material->m_pso->SetHullShader(hs.get());
	}

	// Load geometry shader (optional)
	if (desc->geometryShaderPath.HasPath())
	{
		auto gs = shaderManager.LoadGeometryShader(desc->geometryShaderPath, false);
		shaderState.geometryShader = gs;

		material->m_pso->SetGeometryShader(gs.get());
	}

	// Load pixel shader (optional)
	if (desc->pixelShaderPath.HasPath())
	{
		auto ps = shaderManager.LoadPixelShader(desc->pixelShaderPath, false);
		shaderState.pixelShader = ps;

		material->m_pso->SetPixelShader(ps.get());
	}

	// Finalize the graphics PSO & bind material parameters
	material->SetupPSO(*desc);
	material->m_pso->Finalize();

	material->BindParameters(shaderState);

	material->m_isReady = true;
}