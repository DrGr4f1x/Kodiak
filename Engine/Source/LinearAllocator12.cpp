// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from LinearAllocator.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "LinearAllocator12.h"

#include "CommandListManager12.h"
#include "DeviceManager12.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace std;


namespace Kodiak
{

LinearAllocatorType LinearAllocatorPageManager::s_autoType = LinearAllocatorType::GpuExclusive;
LinearAllocatorPageManager LinearAllocator::s_pageManager[2];

} // namespace Kodiak


LinearAllocatorPageManager::LinearAllocatorPageManager()
{
	m_allocationType = s_autoType;
	s_autoType = (LinearAllocatorType)((int32_t)s_autoType + 1);
	assert(s_autoType <= LinearAllocatorType::NumAllocatorTypes);
}


LinearAllocationPage* LinearAllocatorPageManager::RequestPage()
{
	lock_guard<mutex> LockGuard(m_mutex);

	while (!m_retiredPages.empty() && CommandListManager::GetInstance().IsFenceComplete(m_retiredPages.front().first))
	{
		m_availablePages.push(m_retiredPages.front().second);
		m_retiredPages.pop();
	}

	LinearAllocationPage* pagePtr = nullptr;

	if (!m_availablePages.empty())
	{
		pagePtr = m_availablePages.front();
		m_availablePages.pop();
	}
	else
	{
		pagePtr = CreateNewPage();
		m_pagePool.emplace_back(pagePtr);
	}

	return pagePtr;
}


void LinearAllocatorPageManager::DiscardPages(uint64_t fenceValue, const vector<LinearAllocationPage*>& usedPages)
{
	lock_guard<mutex> LockGuard(m_mutex);
	for (auto iter = usedPages.begin(); iter != usedPages.end(); ++iter)
	{
		m_retiredPages.push(make_pair(fenceValue, *iter));
	}
}


LinearAllocationPage* LinearAllocatorPageManager::CreateNewPage()
{
	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	D3D12_RESOURCE_STATES defaultUsage;

	if (m_allocationType == LinearAllocatorType::GpuExclusive)
	{
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		resourceDesc.Width = kGpuAllocatorPageSize;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		defaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
	else
	{
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		resourceDesc.Width = kCpuAllocatorPageSize;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		defaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	ID3D12Resource* buffer = nullptr;
	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProps, 
		D3D12_HEAP_FLAG_NONE, 
		&resourceDesc,
		defaultUsage, 
		nullptr, 
		IID_PPV_ARGS(&buffer)));

	buffer->SetName(L"LinearAllocator Page");

	return new LinearAllocationPage(buffer, defaultUsage);
}


void LinearAllocator::CleanupUsedPages(uint64_t fenceID)
{
	if (m_curPage == nullptr)
	{
		return;
	}

	m_retiredPages.push_back(m_curPage);
	m_curPage = nullptr;
	m_curOffset = 0;

	s_pageManager[(int32_t)m_allocationType].DiscardPages(fenceID, m_retiredPages);
	m_retiredPages.clear();
}


DynAlloc LinearAllocator::Allocate(size_t sizeInBytes, size_t alignment)
{
	assert(sizeInBytes <= m_pageSize);  //Exceeded max linear allocator page size with single allocation

	const size_t alignmentMask = alignment - 1;

	// Assert that it's a power of two.
	assert((alignmentMask & alignment) == 0);

	// Align the allocation
	const size_t alignedSize = Math::AlignUpWithMask(sizeInBytes, alignmentMask);

	m_curOffset = Math::AlignUp(m_curOffset, alignment);

	if (m_curOffset + alignedSize > m_pageSize)
	{
		assert(m_curPage != nullptr);
		m_retiredPages.push_back(m_curPage);
		m_curPage = nullptr;
	}

	if (m_curPage == nullptr)
	{
		m_curPage = s_pageManager[(int32_t)m_allocationType].RequestPage();
		m_curOffset = 0;
	}

	DynAlloc ret(*m_curPage, m_curOffset, alignedSize);
	ret.dataPtr = (uint8_t*)m_curPage->m_cpuVirtualAddress + m_curOffset;
	ret.gpuAddress = m_curPage->m_gpuVirtualAddress + m_curOffset;

	m_curOffset += alignedSize;

	return ret;
}
