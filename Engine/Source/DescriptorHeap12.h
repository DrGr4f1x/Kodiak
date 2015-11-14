// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from DescriptorHeap.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Kodiak
{

class DeviceManager;

// This is an unbounded resource descriptor allocator.  It is intended to provide space for CPU-visible resource descriptors
// as resources are created.  For those that need to be made shader-visible, they will need to be copied to a UserDescriptorHeap
// or a DynamicDescriptorHeap.
class DescriptorAllocator
{
public:
	DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) : m_type(type), m_currentHeap(nullptr) {}

	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(DeviceManager* deviceManager, uint32_t Count);

	static void DestroyAll();

protected:
	static ID3D12DescriptorHeap* RequestNewHeap(DeviceManager* deviceManager, D3D12_DESCRIPTOR_HEAP_TYPE type);

protected:
	static const uint32_t	s_numDescriptorsPerHeap{ 256 };
	static std::mutex		s_allocationMutex;

	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> s_descriptorHeapPool;
	
	D3D12_DESCRIPTOR_HEAP_TYPE		m_type;
	ID3D12DescriptorHeap*			m_currentHeap{ nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE		m_currentHandle;
	uint32_t						m_descriptorSize{ 0 };
	uint32_t						m_remainingFreeHandles{ s_numDescriptorsPerHeap };
};


class DescriptorHandle
{
public:
	DescriptorHandle()
	{
		m_cpuHandle.ptr = ~0ull;
		m_gpuHandle.ptr = ~0ull;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: m_cpuHandle(cpuHandle)
	{
		m_gpuHandle.ptr = ~0ull;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
		: m_cpuHandle(cpuHandle)
		, m_gpuHandle(gpuHandle)
	{
	}

	DescriptorHandle operator+(int32_t offsetScaledByDescriptorSize) const
	{
		DescriptorHandle ret = *this;
		ret += offsetScaledByDescriptorSize;
		return ret;
	}

	void operator+=(int32_t offsetScaledByDescriptorSize)
	{
		if (m_cpuHandle.ptr != ~0ull)
		{
			m_cpuHandle.ptr += offsetScaledByDescriptorSize;
		}
		if (m_gpuHandle.ptr != ~0ull)
		{
			m_gpuHandle.ptr += offsetScaledByDescriptorSize;
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return m_cpuHandle; }

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return m_gpuHandle; }

	bool IsNull() const { return m_cpuHandle.ptr == ~0ull; }
	bool IsShaderVisible() const { return m_gpuHandle.ptr != ~0ull; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
};


} // namespace Kodiak