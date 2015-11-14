// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from DescriptorHeap.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "DescriptorHeap12.h"

#include "DeviceManager12.h"

using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


namespace Kodiak
{

std::mutex DescriptorAllocator::s_allocationMutex;
vector<ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::s_descriptorHeapPool;

} // namespace Kodiak


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(DeviceManager* deviceManager, uint32_t count)
{
	if (m_currentHeap == nullptr || m_remainingFreeHandles < count)
	{
		m_currentHeap = RequestNewHeap(deviceManager, m_type);
		m_currentHandle = m_currentHeap->GetCPUDescriptorHandleForHeapStart();
		m_remainingFreeHandles = s_numDescriptorsPerHeap;

		if (m_descriptorSize == 0)
		{
			m_descriptorSize = deviceManager->GetDevice()->GetDescriptorHandleIncrementSize(m_type);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ret = m_currentHandle;
	m_currentHandle.ptr += count * m_descriptorSize;
	m_remainingFreeHandles -= count;
	return ret;
}


void DescriptorAllocator::DestroyAll()
{
	s_descriptorHeapPool.clear();
}


ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(DeviceManager* deviceManager, D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
	std::lock_guard<std::mutex> LockGuard(s_allocationMutex);

	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Desc.Type = Type;
	Desc.NumDescriptors = s_numDescriptorsPerHeap;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Desc.NodeMask = 1;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
	assert(S_OK == deviceManager->GetDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&pHeap)));
	s_descriptorHeapPool.emplace_back(pHeap);
	return pHeap.Get();
}