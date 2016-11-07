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

// Forward declarations
enum class ColorFormat;

class Texture : public GpuResource
{
	friend class GraphicsCommandList;

public:
	Texture() {	m_cpuDescriptorHandle.ptr = ~0ull; }
	Texture(D3D12_CPU_DESCRIPTOR_HANDLE handle) : m_cpuDescriptorHandle(handle) {}

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_cpuDescriptorHandle; }

	operator bool() const { return m_cpuDescriptorHandle.ptr != 0 && m_cpuDescriptorHandle.ptr != ~0ull; }
	bool operator!() const { return m_cpuDescriptorHandle.ptr == 0; }

	static std::shared_ptr<Texture> Load(const std::string& path, bool sRGB, bool asyncLoad = true);
	void Create(uint32_t width, uint32_t height, ColorFormat format, const void* initData);
	void CreateArray(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData = nullptr);

	concurrency::task<void> loadTask;

private:
	static void LoadInternal(std::shared_ptr<Texture> texture, bool sRGB, const std::string& path);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE		m_cpuDescriptorHandle;
};

} // namespace Kodiak