// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

class SamplerState
{
public:
	SamplerState(const std::string& name);

	const std::string& GetName() const { return m_name; }

	const D3D12_SAMPLER_DESC& GetDesc() const { return m_desc; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle() const { return m_cpuDescriptorHandle; }

	void Create() { /* TODO */ }

private:
	const std::string				m_name;
	D3D12_SAMPLER_DESC				m_desc;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_cpuDescriptorHandle;
};

} // namespace Kodiak