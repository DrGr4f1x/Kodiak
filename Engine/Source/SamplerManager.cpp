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