// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from CommandListManager.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Kodiak
{

class CommandListManager
{
	friend class CommandList;

public:
	static CommandListManager& GetInstance()
	{
		static CommandListManager instance;
		return instance;
	}

	void Create(ID3D12Device* device);
	void Shutdown();

	inline bool IsReady() { return nullptr != m_commandQueue; }

	uint64_t IncrementFence();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFence(uint64_t fenceValue);
	void IdleGpu() { WaitForFence(IncrementFence()); }

	ID3D12Device* GetDevice() { return m_device; }
	ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue; }

private:
	void CreateNewCommandList(ID3D12GraphicsCommandList** commandList, ID3D12CommandAllocator** allocator);
	uint64_t ExecuteCommandList(ID3D12CommandList* commandList);
	ID3D12CommandAllocator* RequestAllocator();
	void DiscardAllocator(uint64_t fenceValueForReset, ID3D12CommandAllocator* allocator);

private:
	ID3D12Device*		m_device{ nullptr };
	ID3D12CommandQueue*	m_commandQueue{ nullptr };

	std::vector<ID3D12CommandAllocator*>						m_allocatorPool;
	std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>>	m_pendingAllocators;
	std::mutex													m_allocatorMutex;
	std::mutex													m_fenceMutex;
	std::mutex													m_eventMutex;

	ID3D12Fence*	m_fence;
	uint64_t		m_nextFenceValue;
	uint64_t		m_lastCompletedFenceValue;
	HANDLE			m_fenceEventHandle;
};

} // namespace Kodiak
