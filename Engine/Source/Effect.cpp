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


using namespace Kodiak;
using namespace std;


BaseEffect::BaseEffect()
{
	m_topology = PrimitiveTopologyType::Triangle;
	for (uint32_t i = 0; i < 8; ++i)
	{
		m_colorFormats[i] = ColorFormat::Unknown;
	}
	m_numRenderTargets = 0;
	m_depthFormat = DepthFormat::Unknown;
}


BaseEffect::BaseEffect(const string& name) : m_name(name)
{
	m_topology = PrimitiveTopologyType::Triangle;
	for (uint32_t i = 0; i < 8; ++i)
	{
		m_colorFormats[i] = ColorFormat::Unknown;
	}
	m_numRenderTargets = 0;
	m_depthFormat = DepthFormat::Unknown;
}


void BaseEffect::SetVertexShaderPath(const string& shaderPath)
{
	assert(!m_isFinalized);

	m_shaderPaths[0] = shaderPath;
	m_vertexShader = VertexShader::Load(shaderPath);
}


void BaseEffect::SetDomainShaderPath(const string& shaderPath)
{
	assert(!m_isFinalized);

	m_shaderPaths[1] = shaderPath;
	m_domainShader = DomainShader::Load(shaderPath);
}


void BaseEffect::SetHullShaderPath(const string& shaderPath)
{
	assert(!m_isFinalized);

	m_shaderPaths[2] = shaderPath;
	m_hullShader = HullShader::Load(shaderPath);
}


void BaseEffect::SetGeometryShaderPath(const string& shaderPath)
{
	assert(!m_isFinalized);

	m_shaderPaths[3] = shaderPath;
	m_geometryShader = GeometryShader::Load(shaderPath);
}


void BaseEffect::SetPixelShaderPath(const string& shaderPath)
{
	assert(!m_isFinalized);

	m_shaderPaths[4] = shaderPath;
	m_pixelShader = PixelShader::Load(shaderPath);
}


void BaseEffect::SetPrimitiveTopology(PrimitiveTopologyType topology)
{
	assert(!m_isFinalized);
	m_topology = topology;
}


void BaseEffect::SetRenderTargetFormats(uint32_t numRTVs, const ColorFormat* colorFormats, DepthFormat depthFormat, uint32_t msaaCount,
	uint32_t msaaQuality)
{
	assert(numRTVs == 0 || colorFormats != nullptr);
	assert(!m_isFinalized);

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