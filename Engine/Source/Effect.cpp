// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Effect.h"

#include "Material.h"
#include "RenderEnums.h"
#include "ShaderManager.h"


using namespace Kodiak;
using namespace std;


BaseEffect::BaseEffect()
{
	loadTask = concurrency::create_task([] {});

	m_topology = PrimitiveTopologyType::Triangle;
	for (uint32_t i = 0; i < 8; ++i)
	{
		m_colorFormats[i] = ColorFormat::Unknown;
	}
	m_numRenderTargets = 0;
	m_depthFormat = DepthFormat::Unknown;

	m_shaderBindings.reserve(5);
}


BaseEffect::BaseEffect(const string& name) : m_name(name)
{
	loadTask = concurrency::create_task([] {});

	m_topology = PrimitiveTopologyType::Triangle;
	for (uint32_t i = 0; i < 8; ++i)
	{
		m_colorFormats[i] = ColorFormat::Unknown;
	}
	m_numRenderTargets = 0;
	m_depthFormat = DepthFormat::Unknown;

	m_shaderBindings.reserve(5);
}



void BaseEffect::SetVertexShaderPath(const ShaderPath& shaderPath)
{
	m_shaderPaths[0] = shaderPath;
	m_vertexShader = ShaderManager::GetInstance().LoadVertexShader(shaderPath);

	loadTask = loadTask && m_vertexShader->loadTask;
}


void BaseEffect::SetDomainShaderPath(const ShaderPath& shaderPath)
{
	m_shaderPaths[1] = shaderPath;
	m_domainShader = ShaderManager::GetInstance().LoadDomainShader(shaderPath);
	
	loadTask = loadTask && m_domainShader->loadTask;
}


void BaseEffect::SetHullShaderPath(const ShaderPath& shaderPath)
{
	m_shaderPaths[2] = shaderPath;
	m_hullShader = ShaderManager::GetInstance().LoadHullShader(shaderPath);
	
	loadTask = loadTask && m_hullShader->loadTask;
}


void BaseEffect::SetGeometryShaderPath(const ShaderPath& shaderPath)
{
	m_shaderPaths[3] = shaderPath;
	m_geometryShader = ShaderManager::GetInstance().LoadGeometryShader(shaderPath);
	
	loadTask = loadTask && m_geometryShader->loadTask;
}


void BaseEffect::SetPixelShaderPath(const ShaderPath& shaderPath)
{
	m_shaderPaths[4] = shaderPath;
	m_pixelShader = ShaderManager::GetInstance().LoadPixelShader(shaderPath);
	
	loadTask = loadTask && m_pixelShader->loadTask;
}


void BaseEffect::SetPrimitiveTopology(PrimitiveTopologyType topology)
{
	m_topology = topology;
}


void BaseEffect::SetRenderTargetFormats(uint32_t numRTVs, const ColorFormat* colorFormats, DepthFormat depthFormat, uint32_t msaaCount,
	uint32_t msaaQuality)
{
	assert(numRTVs == 0 || colorFormats != nullptr);
	m_numRenderTargets = numRTVs;
	for (uint32_t i = 0; i < numRTVs; ++i)
	{
		m_colorFormats[i] = colorFormats[i];
	}
	for (uint32_t i = numRTVs; i < 8; ++i)
	{
		m_colorFormats[i] = ColorFormat::Unknown;
	}
	m_depthFormat = depthFormat;
	m_msaaCount = msaaCount;
	m_msaaQuality = msaaQuality;
}