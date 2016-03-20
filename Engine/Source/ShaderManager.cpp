// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ShaderManager.h"

#include "Shader.h"

#include <ppltasks.h>

using namespace Kodiak;
using namespace std;


namespace
{

map<size_t, shared_ptr<ComputeShader>>	s_computeShaderHashMap;
map<size_t, shared_ptr<DomainShader>>	s_domainShaderHashMap;
map<size_t, shared_ptr<GeometryShader>> s_geometryShaderHashMap;
map<size_t, shared_ptr<HullShader>>		s_hullShaderHashMap;
map<size_t, shared_ptr<PixelShader>>	s_pixelShaderHashMap;
map<size_t, shared_ptr<VertexShader>>	s_vertexShaderHashMap;

} // anonymous namespace


ShaderManager& ShaderManager::GetInstance()
{
	static ShaderManager instance;
	return instance;
}


void ShaderManager::DestroyAll()
{
	s_computeShaderHashMap.clear();
	s_domainShaderHashMap.clear();
	s_geometryShaderHashMap.clear();
	s_hullShaderHashMap.clear();
	s_pixelShaderHashMap.clear();
	s_vertexShaderHashMap.clear();
}


shared_ptr<ComputeShader> ShaderManager::LoadComputeShader(const ShaderPath& shaderPath, bool asyncLoad) const
{
	string fullPath = shaderPath.GetFullPath();

	hash<string> hashFunc;
	size_t hashCode = hashFunc(fullPath);

	shared_ptr<ComputeShader> computeShader;

	{
		static mutex computeShaderMutex;
		lock_guard<mutex> CS(computeShaderMutex);

		auto iter = s_computeShaderHashMap.find(hashCode);

		if (iter == s_computeShaderHashMap.end())
		{
			computeShader = make_shared<ComputeShader>();
			s_computeShaderHashMap[hashCode] = computeShader;

			if (asyncLoad)
			{
				// Non-blocking asynchronous load
				LoadShaderAsync(computeShader, fullPath);
			}
			else
			{
				// Blocking synchronous load
				LoadShaderSerial(computeShader, fullPath);
			}
		}
		else
		{
			computeShader = iter->second;
		}
	}

	return computeShader;
}


shared_ptr<DomainShader> ShaderManager::LoadDomainShader(const ShaderPath& shaderPath, bool asyncLoad) const
{
	string fullPath = shaderPath.GetFullPath();

	hash<string> hashFunc;
	size_t hashCode = hashFunc(fullPath);

	shared_ptr<DomainShader> domainShader;

	{
		static mutex domainShaderMutex;
		lock_guard<mutex> CS(domainShaderMutex);

		auto iter = s_domainShaderHashMap.find(hashCode);

		if (iter == s_domainShaderHashMap.end())
		{
			domainShader = make_shared<DomainShader>();
			s_domainShaderHashMap[hashCode] = domainShader;

			if (asyncLoad)
			{
				// Non-blocking asynchronous load
				LoadShaderAsync(domainShader, fullPath);
			}
			else
			{
				// Blocking synchronous load
				LoadShaderSerial(domainShader, fullPath);
			}
		}
		else
		{
			domainShader = iter->second;
		}
	}

	return domainShader;
}


shared_ptr<GeometryShader> ShaderManager::LoadGeometryShader(const ShaderPath& shaderPath, bool asyncLoad) const
{
	string fullPath = shaderPath.GetFullPath();

	hash<string> hashFunc;
	size_t hashCode = hashFunc(fullPath);

	shared_ptr<GeometryShader> geometryShader;

	{
		static mutex geometryShaderMutex;
		lock_guard<mutex> CS(geometryShaderMutex);

		auto iter = s_geometryShaderHashMap.find(hashCode);

		if (iter == s_geometryShaderHashMap.end())
		{
			geometryShader = make_shared<GeometryShader>();
			s_geometryShaderHashMap[hashCode] = geometryShader;

			if (asyncLoad)
			{
				// Non-blocking asynchronous load
				LoadShaderAsync(geometryShader, fullPath);
			}
			else
			{
				// Blocking synchronous load
				LoadShaderSerial(geometryShader, fullPath);
			}
		}
		else
		{
			geometryShader = iter->second;
		}
	}

	return geometryShader;
}


shared_ptr<HullShader> ShaderManager::LoadHullShader(const ShaderPath& shaderPath, bool asyncLoad) const
{
	string fullPath = shaderPath.GetFullPath();

	hash<string> hashFunc;
	size_t hashCode = hashFunc(fullPath);

	shared_ptr<HullShader> hullShader;

	{
		static mutex hullShaderMutex;
		lock_guard<mutex> CS(hullShaderMutex);

		auto iter = s_hullShaderHashMap.find(hashCode);

		if (iter == s_hullShaderHashMap.end())
		{
			hullShader = make_shared<HullShader>();
			s_hullShaderHashMap[hashCode] = hullShader;

			if (asyncLoad)
			{
				// Non-blocking asynchronous load
				LoadShaderAsync(hullShader, fullPath);
			}
			else
			{
				// Blocking synchronous load
				LoadShaderSerial(hullShader, fullPath);
			}
		}
		else
		{
			hullShader = iter->second;
		}
	}

	return hullShader;
}


shared_ptr<PixelShader> ShaderManager::LoadPixelShader(const ShaderPath& shaderPath, bool asyncLoad) const
{
	string fullPath = shaderPath.GetFullPath();

	hash<string> hashFunc;
	size_t hashCode = hashFunc(fullPath);

	shared_ptr<PixelShader> pixelShader;

	{
		static mutex pixelShaderMutex;
		lock_guard<mutex> CS(pixelShaderMutex);

		auto iter = s_pixelShaderHashMap.find(hashCode);

		if (iter == s_pixelShaderHashMap.end())
		{
			pixelShader = make_shared<PixelShader>();
			s_pixelShaderHashMap[hashCode] = pixelShader;

			if (asyncLoad)
			{
				// Non-blocking asynchronous load
				LoadShaderAsync(pixelShader, fullPath);
			}
			else
			{
				// Blocking synchronous load
				LoadShaderSerial(pixelShader, fullPath);
			}
		}
		else
		{
			pixelShader = iter->second;
		}
	}

	return pixelShader;
}


shared_ptr<VertexShader> ShaderManager::LoadVertexShader(const ShaderPath& shaderPath, bool asyncLoad) const
{
	string fullPath = shaderPath.GetFullPath();

	hash<string> hashFunc;
	size_t hashCode = hashFunc(fullPath);

	shared_ptr<VertexShader> vertexShader;

	{
		static mutex vertexShaderMutex;
		lock_guard<mutex> CS(vertexShaderMutex);

		auto iter = s_vertexShaderHashMap.find(hashCode);

		if (iter == s_vertexShaderHashMap.end())
		{
			vertexShader = make_shared<VertexShader>();
			s_vertexShaderHashMap[hashCode] = vertexShader;

			if (asyncLoad)
			{
				// Non-blocking asynchronous load
				LoadShaderAsync(vertexShader, fullPath);
			}
			else
			{
				// Blocking synchronous load
				LoadShaderSerial(vertexShader, fullPath);
			}
		}
		else
		{
			vertexShader = iter->second;
		}
	}

	return vertexShader;
}