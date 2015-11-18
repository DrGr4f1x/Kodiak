// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from LinearAllocator.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "GpuResource.h"

// Constant blocks must be multiples of 16 constants @ 16 bytes each
#define DEFAULT_ALIGN 256

namespace Kodiak
{

// Various types of allocations may contain NULL pointers.Check before dereferencing if you are unsure.
struct DynAlloc
{
	DynAlloc(GpuResource& baseResource, size_t thisOffset, size_t thisSize)
		: buffer(baseResource), offset(thisOffset), size(thisSize) {}

	GpuResource&	buffer;		// The D3D buffer associated with this memory.
	size_t			offset;		// Offset from start of buffer resource
	size_t			size;		// Reserved size of this allocation
	void*			dataPtr;	// The CPU-writeable address
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;	// The GPU-visible address
};


class LinearAllocationPage : public GpuResource
{
public:
	LinearAllocationPage(ID3D12Resource* resource, D3D12_RESOURCE_STATES usage) : GpuResource()
	{
		m_resource.Attach(resource);
		m_usageState = usage;
		m_gpuVirtualAddress = m_resource->GetGPUVirtualAddress();
		m_resource->Map(0, nullptr, &m_cpuVirtualAddress);
	}

	~LinearAllocationPage()
	{
		m_resource->Unmap(0, nullptr);
	}

	void*						m_cpuVirtualAddress;
	D3D12_GPU_VIRTUAL_ADDRESS	m_gpuVirtualAddress;
};


enum class LinearAllocatorType
{
	InvalidAllocator = -1,

	GpuExclusive = 0,		// DEFAULT   GPU-writeable (via UAV)
	CpuWritable = 1,		// UPLOAD    CPU-writeable (but write combined)

	NumAllocatorTypes
};


enum
{
	kGpuAllocatorPageSize = 0x10000,	// 64K
	kCpuAllocatorPageSize = 0x200000	// 2MB
};


class LinearAllocatorPageManager
{
public:

	LinearAllocatorPageManager();
	LinearAllocationPage* RequestPage();
	void DiscardPages(uint64_t fenceID, const std::vector<LinearAllocationPage*>& pages);

	void Destroy(void) { m_pagePool.clear(); }

private:

	LinearAllocationPage* CreateNewPage();

	static LinearAllocatorType s_autoType;

	LinearAllocatorType m_allocationType;
	std::vector<std::unique_ptr<LinearAllocationPage>> m_pagePool;
	std::queue<std::pair<uint64_t, LinearAllocationPage*>> m_retiredPages;
	std::queue<LinearAllocationPage*> m_availablePages;
	std::mutex m_mutex;
};


class LinearAllocator
{
public:

	LinearAllocator(LinearAllocatorType type) : m_allocationType(type), m_pageSize(0), m_curOffset(~0ull), m_curPage(nullptr)
	{
		assert(type > LinearAllocatorType::InvalidAllocator && type < LinearAllocatorType::NumAllocatorTypes);
		m_pageSize = (type == LinearAllocatorType::GpuExclusive ? kGpuAllocatorPageSize : kCpuAllocatorPageSize);
	}

	DynAlloc Allocate(size_t sizeInBytes, size_t alignment = DEFAULT_ALIGN);

	void CleanupUsedPages(uint64_t fenceID);

	static void DestroyAll()
	{
		s_pageManager[0].Destroy();
		s_pageManager[1].Destroy();
	}

private:

	static LinearAllocatorPageManager s_pageManager[2];

	LinearAllocatorType m_allocationType;
	size_t m_pageSize;
	size_t m_curOffset;
	LinearAllocationPage* m_curPage;
	std::vector<LinearAllocationPage*> m_retiredPages;
};


} // namespace Kodiak