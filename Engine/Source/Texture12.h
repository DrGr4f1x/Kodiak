// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "GpuResource12.h"

#include <ppltasks.h>

namespace Kodiak
{

class Texture : public GpuResource
{
	friend class GraphicsCommandList;

public:
	Texture() {	m_cpuDescriptorHandle.ptr = ~0ull; }
	Texture(D3D12_CPU_DESCRIPTOR_HANDLE handle) : m_cpuDescriptorHandle(handle) {}

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_cpuDescriptorHandle; }

	bool operator!() { return m_cpuDescriptorHandle.ptr == 0; }

	static std::shared_ptr<Texture> Load(const std::string& path, bool sRGB, bool asyncLoad = true);

	concurrency::task<void> loadTask;

private:
	static void LoadInternal(std::shared_ptr<Texture> texture, bool sRGB, const std::string& path);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE		m_cpuDescriptorHandle;
};

} // namespace Kodiak