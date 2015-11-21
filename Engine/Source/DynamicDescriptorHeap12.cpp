// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from DynamicDescriptorHeap.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "DynamicDescriptorHeap12.h"

#include "CommandList12.h"
#include "CommandListManager12.h"
#include "DeviceManager12.h"
#include "RenderUtils.h"

#include <intrin.h>


using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;


#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanForward64)


namespace Kodiak
{

	mutex DynamicDescriptorHeap::s_mutex;
	vector<ComPtr<ID3D12DescriptorHeap>> DynamicDescriptorHeap::s_descriptorHeapPool;
	queue<pair<uint64_t, ID3D12DescriptorHeap*>> DynamicDescriptorHeap::s_retiredDescriptorHeaps;
	queue<ID3D12DescriptorHeap*> DynamicDescriptorHeap::s_availableDescriptorHeaps;
	uint32_t DynamicDescriptorHeap::s_descriptorSize = 0;

} // namespace Kodiak


DynamicDescriptorHeap::DynamicDescriptorHeap(CommandList& owner) : m_owningCommandList(owner) {}


void DynamicDescriptorHeap::DestroyAll()
{
	s_descriptorHeapPool.clear();
}


void DynamicDescriptorHeap::CleanupUsedHeaps(uint64_t fenceValue)
{
	RetireCurrentHeap();
	RetireUsedHeaps(fenceValue);
	m_graphicsHandleCache.ClearCache();
	m_computeHandleCache.ClearCache();
}


D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	if(!HasSpace(1))
	{
		RetireCurrentHeap();
		UnbindAllValid();
	}

	m_owningCommandList.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GetHeapPointer());

	DescriptorHandle DestHandle = m_firstDescriptor + m_currentOffset * GetDescriptorSize();
	m_currentOffset += 1;

	g_device->CopyDescriptorsSimple(1, DestHandle.GetCpuHandle(), handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return DestHandle.GetGpuHandle();
}


uint32_t DynamicDescriptorHeap::GetDescriptorSize()
{
	if (s_descriptorSize == 0)
	{
		s_descriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	return s_descriptorSize;
}


ID3D12DescriptorHeap* DynamicDescriptorHeap::RequestDescriptorHeap()
{
	lock_guard<mutex> LockGuard(s_mutex);

	while(!s_retiredDescriptorHeaps.empty() && CommandListManager::GetInstance().IsFenceComplete(s_retiredDescriptorHeaps.front().first))
	{
		s_availableDescriptorHeaps.push(s_retiredDescriptorHeaps.front().second);
		s_retiredDescriptorHeaps.pop();
	}

	if(!s_availableDescriptorHeaps.empty())
	{
		ID3D12DescriptorHeap* HeapPtr = s_availableDescriptorHeaps.front();
		s_availableDescriptorHeaps.pop();
		return HeapPtr;
	}
	else
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = kNumDescriptorsPerHeap;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.NodeMask = 1;
		ComPtr<ID3D12DescriptorHeap> heapPtr;
		ThrowIfFailed(g_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapPtr)));
		s_descriptorHeapPool.emplace_back(heapPtr);
		return heapPtr.Get();
	}
}


void DynamicDescriptorHeap::DiscardDescriptorHeaps(uint64_t fenceValue, const std::vector<ID3D12DescriptorHeap*>& usedHeaps)
{
	lock_guard<mutex> LockGuard(s_mutex);
	for (auto iter = usedHeaps.begin(); iter != usedHeaps.end(); ++iter)
	{
		s_retiredDescriptorHeaps.push(make_pair(fenceValue, *iter));
	}
}


void DynamicDescriptorHeap::RetireCurrentHeap()
{
	// Don't retire unused heaps.
	if (m_currentOffset == 0)
	{
		assert(m_currentHeapPtr == nullptr);
		return;
	}

	assert(m_currentHeapPtr != nullptr);
	m_retiredHeaps.push_back(m_currentHeapPtr);
	m_currentHeapPtr = nullptr;
	m_currentOffset = 0;
}


void DynamicDescriptorHeap::RetireUsedHeaps(uint64_t fenceValue)
{
	DiscardDescriptorHeaps(fenceValue, m_retiredHeaps);
	m_retiredHeaps.clear();
}


inline ID3D12DescriptorHeap* DynamicDescriptorHeap::GetHeapPointer()
{
	if (m_currentHeapPtr == nullptr)
	{
		assert(m_currentOffset == 0);
		m_currentHeapPtr = RequestDescriptorHeap();
		m_firstDescriptor = DescriptorHandle(
			m_currentHeapPtr->GetCPUDescriptorHandleForHeapStart(),
			m_currentHeapPtr->GetGPUDescriptorHandleForHeapStart());
	}

	return m_currentHeapPtr;
}


void DynamicDescriptorHeap::UnbindAllValid()
{
	m_graphicsHandleCache.UnbindAllValid();
	m_computeHandleCache.UnbindAllValid();
}


uint32_t DynamicDescriptorHeap::DescriptorHandleCache::ComputeStagedSize()
{
	// Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
	uint32_t neededSpace = 0;
	uint32_t rootIndex;
	uint32_t staleParams = staleRootParamsBitMap;
	while(_BitScanForward((unsigned long*)&rootIndex, staleParams))
	{
		staleParams ^= (1 << rootIndex);

		uint32_t maxSetHandle;

		assert(TRUE == _BitScanReverse((unsigned long*)&maxSetHandle, rootDescriptorTable[rootIndex].assignedHandlesBitMap));
		// Root entry marked as stale but has no stale descriptors

		neededSpace += maxSetHandle + 1;
	}
	return neededSpace;
}


void DynamicDescriptorHeap::DescriptorHandleCache::CopyAndBindStaleTables(
	DescriptorHandle destHandleStart, ID3D12GraphicsCommandList* cmdList,
	void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
	uint32_t staleParamCount = 0;
	uint32_t tableSize[DescriptorHandleCache::kMaxNumDescriptorTables];
	uint32_t rootIndices[DescriptorHandleCache::kMaxNumDescriptorTables];
	uint32_t neededSpace = 0;
	uint32_t rootIndex;

	// Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
	uint32_t staleParams = staleRootParamsBitMap;
	while (_BitScanForward((unsigned long*)&rootIndex, staleParams))
	{
		rootIndices[staleParamCount] = rootIndex;
		staleParams ^= (1 << rootIndex);

		uint32_t maxSetHandle;

		assert(TRUE == _BitScanReverse((unsigned long*)&maxSetHandle, rootDescriptorTable[rootIndex].assignedHandlesBitMap));
		// Root entry marked as stale but has no stale descriptors

		neededSpace += maxSetHandle + 1;
		tableSize[staleParamCount] = maxSetHandle + 1;

		++staleParamCount;
	}

	assert(staleParamCount <= DescriptorHandleCache::kMaxNumDescriptorTables);
	// We're only equipped to handle so many descriptor tables

	staleRootParamsBitMap = 0;

	static const uint32_t kMaxDescriptorsPerCopy = 16;
	uint32_t numDestDescriptorRanges = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[kMaxDescriptorsPerCopy];
	uint32_t pDestDescriptorRangeSizes[kMaxDescriptorsPerCopy];

	uint32_t numSrcDescriptorRanges = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE pSrcDescriptorRangeStarts[kMaxDescriptorsPerCopy];
	uint32_t pSrcDescriptorRangeSizes[kMaxDescriptorsPerCopy];

	const uint32_t kDescriptorSize = DynamicDescriptorHeap::GetDescriptorSize();

	for (uint32_t i = 0; i < staleParamCount; ++i)
	{
		rootIndex = rootIndices[i];
		(cmdList->*SetFunc)(rootIndex, destHandleStart.GetGpuHandle());

		DescriptorTableCache& rootDescTable = rootDescriptorTable[rootIndex];

		D3D12_CPU_DESCRIPTOR_HANDLE* srcHandles = rootDescTable.tableStart;
		uint64_t setHandles = (uint64_t)rootDescTable.assignedHandlesBitMap;
		D3D12_CPU_DESCRIPTOR_HANDLE curDest = destHandleStart.GetCpuHandle();
		destHandleStart += tableSize[i] * kDescriptorSize;

		unsigned long skipCount;
		while (_BitScanForward64(&skipCount, setHandles))
		{
			// Skip over unset descriptor handles
			setHandles >>= skipCount;
			srcHandles += skipCount;
			curDest.ptr += skipCount * kDescriptorSize;

			unsigned long descriptorCount;
			_BitScanForward64(&descriptorCount, ~setHandles);
			setHandles >>= descriptorCount;

			// If we run out of temp room, copy what we've got so far
			if (numSrcDescriptorRanges + descriptorCount > kMaxDescriptorsPerCopy)
			{
				g_device->CopyDescriptors(
					numDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
					numSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				numSrcDescriptorRanges = 0;
				numDestDescriptorRanges = 0;
			}

			// Setup destination range
			pDestDescriptorRangeStarts[numDestDescriptorRanges] = curDest;
			pDestDescriptorRangeSizes[numDestDescriptorRanges] = descriptorCount;
			++numDestDescriptorRanges;

			// Setup source ranges (one descriptor each because we don't assume they are contiguous)
			for (uint32_t i = 0; i < descriptorCount; ++i)
			{
				pSrcDescriptorRangeStarts[numSrcDescriptorRanges] = srcHandles[i];
				pSrcDescriptorRangeSizes[numSrcDescriptorRanges] = 1;
				++numSrcDescriptorRanges;
			}

			// Move the destination pointer forward by the number of descriptors we will copy
			srcHandles += descriptorCount;
			curDest.ptr += descriptorCount * kDescriptorSize;
		}
	}

	g_device->CopyDescriptors(
		numDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
		numSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}


void DynamicDescriptorHeap::DescriptorHandleCache::UnbindAllValid()
{
	staleRootParamsBitMap = 0;

	unsigned long tableParams = rootDescriptorTablesBitMap;
	unsigned long rootIndex;
	while(_BitScanForward(&rootIndex, tableParams))
	{
		tableParams ^= (1 << rootIndex);
		if (rootDescriptorTable[rootIndex].assignedHandlesBitMap != 0)
		{
			staleRootParamsBitMap |= (1 << rootIndex);
		}
	}
}


void DynamicDescriptorHeap::DescriptorHandleCache::StageDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const D3D12_CPU_DESCRIPTOR_HANDLE handles[])
{
	assert(((1 << rootIndex) & rootDescriptorTablesBitMap) != 0); // Root parameter is not a CBV_SRV_UAV descriptor table"
	assert(offset + numHandles <= rootDescriptorTable[rootIndex].tableSize);

	DescriptorTableCache& tableCache = rootDescriptorTable[rootIndex];
	D3D12_CPU_DESCRIPTOR_HANDLE* copyDest = tableCache.tableStart + offset;
	for (uint32_t i = 0; i < numHandles; ++i)
	{
		copyDest[i] = handles[i];
	}
	tableCache.assignedHandlesBitMap |= ((1 << numHandles) - 1) << offset;
	staleRootParamsBitMap |= (1 << rootIndex);
}


void DynamicDescriptorHeap::DescriptorHandleCache::ParseRootSignature(const RootSignature& rootSig)
{
	uint32_t currentOffset = 0;

	assert(rootSig.m_numParameters <= 16); // Maybe we need to support something greater

	staleRootParamsBitMap = 0;
	rootDescriptorTablesBitMap = rootSig.m_descriptorTableBitMap;

	unsigned long tableParams = rootDescriptorTablesBitMap;
	unsigned long rootIndex;
	while(_BitScanForward(&rootIndex, tableParams))
	{
		tableParams ^= (1 << rootIndex);

		uint32_t tableSize = rootSig.m_descriptorTableSize[rootIndex];
		assert(tableSize > 0);

		DescriptorTableCache& rootDescTable = rootDescriptorTable[rootIndex];
		rootDescTable.assignedHandlesBitMap = 0;
		rootDescTable.tableStart = handleCache + currentOffset;
		rootDescTable.tableSize = tableSize;

		currentOffset += tableSize;
	}

	maxCachedDescriptors = currentOffset;

	assert(maxCachedDescriptors <= kMaxNumDescriptors); //Exceeded user-supplied maximum cache size
}


void DynamicDescriptorHeap::CopyAndBindStagedTables(DescriptorHandleCache& handleCache, ID3D12GraphicsCommandList* cmdList,
	void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
	uint32_t neededSize = handleCache.ComputeStagedSize();
	if(!HasSpace(neededSize))
	{
		RetireCurrentHeap();
		UnbindAllValid();
	}

	// This can trigger the creation of a new heap
	m_owningCommandList.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GetHeapPointer());

	handleCache.CopyAndBindStaleTables(Allocate(neededSize), cmdList, SetFunc);
}