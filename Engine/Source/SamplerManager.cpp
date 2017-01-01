// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "SamplerManager.h"

using namespace Kodiak;
using namespace std;


void SamplerManager::Initialize()
{
	// Anisotropic wrap
	{
		auto sampler = make_shared<SamplerState>("AnisotropicWrap");
		m_samplers.push_back(sampler);
	}

#if DX12
	// Linear clamp
	{
		auto sampler = make_shared<SamplerState>("LinearSampler");
		sampler->m_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler->m_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler->m_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler->m_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		m_samplers.push_back(sampler);
	}

	// Point clamp
	{
		auto sampler = make_shared<SamplerState>("PointSampler");
		sampler->m_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler->m_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler->m_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler->m_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		m_samplers.push_back(sampler);
	}

	// Linear border
	{
		auto sampler = make_shared<SamplerState>("LinearBorderSampler");
		sampler->m_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler->m_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler->m_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler->m_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler->m_desc.BorderColor[0] = 0.0f;
		sampler->m_desc.BorderColor[1] = 0.0f;
		sampler->m_desc.BorderColor[2] = 0.0f;
		sampler->m_desc.BorderColor[3] = 0.0f;
		m_samplers.push_back(sampler);
	}

	// Point border
	{
		auto sampler = make_shared<SamplerState>("PointBorderSampler");
		sampler->m_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler->m_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler->m_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler->m_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler->m_desc.BorderColor[0] = 0.0f;
		sampler->m_desc.BorderColor[1] = 0.0f;
		sampler->m_desc.BorderColor[2] = 0.0f;
		sampler->m_desc.BorderColor[3] = 0.0f;
		m_samplers.push_back(sampler);
	}

	// Shadow
	{
		auto sampler = make_shared<SamplerState>("ShadowSampler");

		sampler->m_desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		sampler->m_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler->m_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler->m_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler->m_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		sampler->m_desc.BorderColor[0] = 0.0f;
		sampler->m_desc.BorderColor[1] = 0.0f;
		sampler->m_desc.BorderColor[2] = 0.0f;
		sampler->m_desc.BorderColor[3] = 0.0f;
		m_samplers.push_back(sampler);
	}
#endif

	// Create all states
	for (auto& sampler : m_samplers)
	{
		sampler->Create();
	}
}


void SamplerManager::Shutdown()
{
	m_samplers.clear();
}


bool SamplerManager::IsBuiltInSamplerState(const std::string& name) const
{
	for (const auto& sampler : m_samplers)
	{
		if (sampler->GetName() == name)
		{
			return true;
		}
	}

	return false;
}


std::shared_ptr<SamplerState> SamplerManager::GetSamplerState(const std::string& name) const
{
	for (const auto& sampler : m_samplers)
	{
		if (sampler->GetName() == name)
		{
			return sampler;
		}
	}

	return nullptr;
}