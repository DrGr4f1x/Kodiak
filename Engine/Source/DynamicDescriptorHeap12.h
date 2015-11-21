// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from DynamicDescriptorHeap.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "DescriptorHeap12.h"
#include "RootSignature12.h"

namespace Kodiak
{

// Forward declarations
class CommandList;

// This class is a linear allocation system for dynamically generated descriptor tables.  It internally caches
// CPU descriptor handles so that when not enough space is available in the current heap, necessary descriptors
// can be re-copied to the new heap.
class DynamicDescriptorHeap
{
public:
	DynamicDescriptorHeap(CommandList& owner);

	static void DestroyAll();

	void CleanupUsedHeaps(uint64_t fenceValue);

	// Copy multiple handles into the cache area reserved for the specified root parameter.
	void SetGraphicsDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const D3D12_CPU_DESCRIPTOR_HANDLE handles[])
	{
		m_graphicsHandleCache.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
	}

	void SetComputeDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const D3D12_CPU_DESCRIPTOR_HANDLE handles[])
	{
		m_computeHandleCache.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
	}

	// Bypass the cache and upload directly to the shader-visible heap
	D3D12_GPU_DESCRIPTOR_HANDLE UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE handles);

	// Deduce cache layout needed to support the descriptor tables needed by the root signature.
	void ParseGraphicsRootSignature(const RootSignature& rootSig)
	{
		m_graphicsHandleCache.ParseRootSignature(rootSig);
	}

	void ParseComputeRootSignature(const RootSignature& rootSig)
	{
		m_computeHandleCache.ParseRootSignature(rootSig);
	}

	// Upload any new descriptors in the cache to the shader-visible heap.
	inline void CommitGraphicsRootDescriptorTables(ID3D12GraphicsCommandList* cmdList)
	{
		if (m_graphicsHandleCache.staleRootParamsBitMap != 0)
		{
			CopyAndBindStagedTables(m_graphicsHandleCache, cmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
		}
	}

	inline void CommitComputeRootDescriptorTables(ID3D12GraphicsCommandList* cmdList)
	{
		if (m_computeHandleCache.staleRootParamsBitMap != 0)
		{
			CopyAndBindStagedTables(m_computeHandleCache, cmdList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
		}
	}

	static uint32_t GetDescriptorSize();

private:
	// Static methods
	static ID3D12DescriptorHeap* RequestDescriptorHeap();
	static void DiscardDescriptorHeaps(uint64_t fenceValueForReset, const std::vector<ID3D12DescriptorHeap*>& usedHeaps);

	bool HasSpace(uint32_t count)
	{
		return (m_currentHeapPtr != nullptr && m_currentOffset + count <= kNumDescriptorsPerHeap);
	}

	void RetireCurrentHeap();
	void RetireUsedHeaps(uint64_t fenceValue);
	ID3D12DescriptorHeap* GetHeapPointer();

	DescriptorHandle Allocate(uint32_t count)
	{
		DescriptorHandle ret = m_firstDescriptor + m_currentOffset * GetDescriptorSize();
		m_currentOffset += count;
		return ret;
	}

	// Mark all descriptors in the cache as stale and in need of re-uploading.
	void UnbindAllValid(void);

private:
	// Static members
	static const uint32_t kNumDescriptorsPerHeap = 1024;
	static std::mutex s_mutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> s_descriptorHeapPool;
	static std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> s_retiredDescriptorHeaps;
	static std::queue<ID3D12DescriptorHeap*> s_availableDescriptorHeaps;
	static uint32_t s_descriptorSize;

	// Non-static members
	CommandList& m_owningCommandList;
	ID3D12DescriptorHeap* m_currentHeapPtr{ nullptr };
	uint32_t m_currentOffset{ 0 };
	DescriptorHandle m_firstDescriptor;
	std::vector<ID3D12DescriptorHeap*> m_retiredHeaps;

	// Describes a descriptor table entry:  a region of the handle cache and which handles have been set
	struct DescriptorTableCache
	{
		uint32_t assignedHandlesBitMap{ 0 };
		D3D12_CPU_DESCRIPTOR_HANDLE* tableStart{ nullptr };
		uint32_t tableSize{ 0 };
	};

	struct DescriptorHandleCache
	{
		void ClearCache()
		{
			rootDescriptorTablesBitMap = 0;
			maxCachedDescriptors = 0;
		}

		uint32_t rootDescriptorTablesBitMap{ 0 };
		uint32_t staleRootParamsBitMap{ 0 };
		uint32_t maxCachedDescriptors{ 0 };

		static const uint32_t kMaxNumDescriptors = 256;
		static const uint32_t kMaxNumDescriptorTables = 16;

		uint32_t ComputeStagedSize();
		void CopyAndBindStaleTables(DescriptorHandle destHandleStart, ID3D12GraphicsCommandList* cmdList,
			void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

		DescriptorTableCache rootDescriptorTable[kMaxNumDescriptorTables];
		D3D12_CPU_DESCRIPTOR_HANDLE handleCache[kMaxNumDescriptors];

		void UnbindAllValid();
		void StageDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const D3D12_CPU_DESCRIPTOR_HANDLE handles[]);
		void ParseRootSignature(const RootSignature& rootSig);
	};

	DescriptorHandleCache m_graphicsHandleCache;
	DescriptorHandleCache m_computeHandleCache;

	void CopyAndBindStagedTables(DescriptorHandleCache& handleCache, ID3D12GraphicsCommandList* cmdList,
		void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));
};

} // namespace Kodiak