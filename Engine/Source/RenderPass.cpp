// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "RenderPass.h"

#include "Format.h"

using namespace Kodiak;
using namespace std;


RenderPass::RenderPass(const string& name)
	: m_name(name)
	, m_hash(0)
	, m_depthFormat(DepthFormat::Unknown)
	, m_numRenderTargets(0)
	, m_msaaCount(1)
	, m_msaaQuality(1)
{
	hash<string> hashFunc;
	m_hash = hashFunc(m_name);

	for (uint32_t i = 0; i < 8; ++i)
	{
		m_colorFormats[i] = ColorFormat::Unknown;
	}
}


void RenderPass::SetRenderTargetFormats(uint32_t numRTVs, const ColorFormat* colorFormat, DepthFormat depthFormat, uint32_t msaaCount, uint32_t msaaQuality)
{
	m_depthFormat = depthFormat;

	for (uint32_t i = 0; i < numRTVs; ++i)
	{
		m_colorFormats[i] = colorFormat[i];
	}
	for (uint32_t i = numRTVs; i < 8; ++i)
	{
		m_colorFormats[i] = ColorFormat::Unknown;
	}

	m_msaaCount = msaaCount;
	m_msaaQuality = msaaQuality;
}


void RenderPass::GetRenderTargetFormat(ColorFormat& colorFormat, DepthFormat& depthFormat, uint32_t& msaaCount, uint32_t& msaaQuality)
{
	colorFormat = m_colorFormats[0];
	depthFormat = m_depthFormat;
	msaaCount = m_msaaCount;
	msaaQuality = m_msaaQuality;
}


void RenderPass::GetRenderTargetFormats(uint32_t numRTVs, ColorFormat** colorFormats, DepthFormat& depthFormat, uint32_t& msaaCount,
	uint32_t& msaaQuality)
{
	for (uint32_t i = 0; i < numRTVs; ++i)
	{
		*colorFormats[i] = m_colorFormats[i];
	}

	depthFormat = m_depthFormat;
	msaaCount = m_msaaCount;
	msaaQuality = m_msaaQuality;
}