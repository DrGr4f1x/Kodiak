// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "Sampler.h"

namespace Kodiak
{

class SamplerManager
{
public:
	static SamplerManager& GetInstance()
	{
		static SamplerManager instance;
		return instance;
	}

	void Initialize();
	void Shutdown();

	bool IsBuiltInSamplerState(const std::string& name) const;
	std::shared_ptr<SamplerState> GetSamplerState(const std::string& name) const;

private:
	std::vector<std::shared_ptr<SamplerState>> m_samplers;
};

} // namespace Kodiak