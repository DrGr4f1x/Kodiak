// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from CommandListManager.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "CommandListManager12.h"

#include "RenderUtils.h"


using namespace Kodiak;
using namespace std;


void CommandListManager::Create(ID3D12Device* device)
{
	assert(nullptr != device);
	assert(!IsReady());
	assert(m_allocatorPool.empty());

	m_device = device;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.NodeMask = 1;
	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	m_commandQueue->SetName(L"CommandListManager::m_commandQueue");

	m_nextFenceValue = 1;
	m_lastCompletedFenceValue = 0;
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fence->SetName(L"CommandListManager::m_fence");
	m_fence->Signal(0);

	m_fenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	
	assert(IsReady());
}


void CommandListManager::Shutdown()
{
	if (nullptr == m_commandQueue)
	{
		return;
	}

	CloseHandle(m_fenceEventHandle);

	for (size_t i = 0; i < m_allocatorPool.size(); ++i)
	{
		m_allocatorPool[i]->Release();
		m_allocatorPool[i] = nullptr;
	}

	m_fence->Release();
	m_fence = nullptr;

	m_commandQueue->Release();
	m_commandQueue = nullptr;
}


uint64_t CommandListManager::IncrementFence()
{
	lock_guard<mutex> lockGuard(m_fenceMutex);
	m_commandQueue->Signal(m_fence, m_nextFenceValue);
	return m_nextFenceValue++;
}


bool CommandListManager::IsFenceComplete(uint64_t fenceValue)
{
	// Avoid querying the fence value by testing against the last one seen.
	// The max() is to protect against an unlikely race condition that could cause the last
	// completed fence value to regress.
	if (fenceValue > m_lastCompletedFenceValue)
		m_lastCompletedFenceValue = max(m_lastCompletedFenceValue, m_fence->GetCompletedValue());

	return fenceValue <= m_lastCompletedFenceValue;
}


void CommandListManager::WaitForFence(uint64_t fenceValue)
{
	if (IsFenceComplete(fenceValue))
	{
		return;
	}

	// TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
	// wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
	// the fence can only have one event set on completion, then thread B has to wait for 
	// 100 before it knows 99 is ready.  Maybe insert sequential events?
	{
		lock_guard<mutex> LockGuard(m_eventMutex);

		m_fence->SetEventOnCompletion(fenceValue, m_fenceEventHandle);
		WaitForSingleObject(m_fenceEventHandle, INFINITE);
		m_lastCompletedFenceValue = fenceValue;
	}
}


void CommandListManager::CreateNewCommandList(ID3D12GraphicsCommandList** commandList, ID3D12CommandAllocator** allocator)
{
	*allocator = RequestAllocator();
	ThrowIfFailed(m_device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, *allocator, nullptr, IID_PPV_ARGS(commandList)));
	(*commandList)->SetName(L"CommandList");
}


uint64_t CommandListManager::ExecuteCommandList(ID3D12CommandList* commandList)
{
	lock_guard<mutex> LockGuard(m_fenceMutex);

	// Kickoff the command list
	m_commandQueue->ExecuteCommandLists(1, &commandList);

	// Signal the next fence value (with the GPU)
	m_commandQueue->Signal(m_fence, m_nextFenceValue);

	// And increment the fence value.  
	return m_nextFenceValue++;
}


ID3D12CommandAllocator* CommandListManager::RequestAllocator()
{
	lock_guard<mutex> lockGuard(m_allocatorMutex);

	ID3D12CommandAllocator* allocator = nullptr;

	if (!m_pendingAllocators.empty())
	{
		auto& allocatorPair = m_pendingAllocators.front();

		if (IsFenceComplete(allocatorPair.first))
		{
			allocator = allocatorPair.second;
			ThrowIfFailed(allocator->Reset());
			m_pendingAllocators.pop();
		}
	}

	if (nullptr == allocator)
	{
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
		wchar_t allocatorName[32];
		swprintf(allocatorName, 32, L"CommandAllocator %zu", m_allocatorPool.size());
		allocator->SetName(allocatorName);
		m_allocatorPool.push_back(allocator);
	}

	return allocator;
}


void CommandListManager::DiscardAllocator(uint64_t fenceValueForReset, ID3D12CommandAllocator* allocator)
{
	lock_guard<mutex> lockGuard(m_allocatorMutex);

	// Fence value indicates it is safe to reset the allocator
	m_pendingAllocators.push(make_pair(fenceValueForReset, allocator));
}