// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from CommandContext.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "CommandList12.h"

#include "ColorBuffer12.h"
#include "CommandListManager12.h"
#include "DepthBuffer12.h"
#include "DeviceResources12.h"
#include "GpuResource12.h"
#include "PipelineState12.h"
#include "Rectangle.h"
#include "RenderUtils.h"
#include "RootSignature12.h"
#include "Viewport.h"


using namespace Kodiak;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;


vector<std::unique_ptr<CommandList>> CommandList::s_commandListPool;
queue<CommandList*> CommandList::s_availableCommandLists;
mutex CommandList::s_commandListAllocationMutex;


CommandList::CommandList()
	: m_dynamicDescriptorHeap(*this)
{
	ZeroMemory(m_currentDescriptorHeaps, sizeof(m_currentDescriptorHeaps));
}


CommandList::~CommandList()
{
	if (nullptr != m_commandList)
	{
		m_commandList->Release();
	}
}


void CommandList::DestroyAllCommandLists()
{
	s_commandListPool.clear();
}


CommandList& CommandList::Begin()
{
	CommandList* newCommandList = CommandList::AllocateCommandList();
	return *newCommandList;
}


uint64_t CommandList::CloseAndExecute(bool waitForCompletion)
{
	uint64_t fence = Finish(waitForCompletion);
	FreeCommandList(this);
	return fence;
}


void CommandList::Initialize(CommandListManager& manager)
{
	m_owner = &manager;
	m_owner->CreateNewCommandList(&m_commandList, &m_currentAllocator);
}


void CommandList::TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
{
	D3D12_RESOURCE_STATES oldState = resource.m_usageState;

	if (oldState != newState)
	{
		assert(m_numBarriersToFlush < 16);
		D3D12_RESOURCE_BARRIER& BarrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Transition.pResource = resource.GetResource();
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = oldState;
		BarrierDesc.Transition.StateAfter = newState;

		// Check to see if we already started the transition
		if (newState == resource.m_transitioningState)
		{
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			resource.m_transitioningState = (D3D12_RESOURCE_STATES)-1;
		}
		else
		{
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		}

		resource.m_usageState = newState;
	}
	else if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	{
		InsertUAVBarrier(resource, flushImmediate);
	}

	if (flushImmediate || m_numBarriersToFlush == 16)
	{
		m_commandList->ResourceBarrier(m_numBarriersToFlush, m_resourceBarrierBuffer);
		m_numBarriersToFlush = 0;
	}
}


void CommandList::BeginResourceTransition(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
{
	// If it's already transitioning, finish that transition
	if (resource.m_transitioningState != (D3D12_RESOURCE_STATES)-1)
	{
		TransitionResource(resource, resource.m_transitioningState);
	}

	D3D12_RESOURCE_STATES oldState = resource.m_usageState;

	if (oldState != newState)
	{
		assert(m_numBarriersToFlush < 16);
		D3D12_RESOURCE_BARRIER& BarrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Transition.pResource = resource.GetResource();
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = oldState;
		BarrierDesc.Transition.StateAfter = newState;

		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

		resource.m_transitioningState = newState;
	}

	if (flushImmediate || m_numBarriersToFlush == 16)
	{
		m_commandList->ResourceBarrier(m_numBarriersToFlush, m_resourceBarrierBuffer);
		m_numBarriersToFlush = 0;
	}
}


void CommandList::InsertUAVBarrier(GpuResource& resource, bool flushImmediate)
{
	assert(m_numBarriersToFlush < 16);
	D3D12_RESOURCE_BARRIER& BarrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.UAV.pResource = resource.GetResource();

	if (flushImmediate)
	{
		m_commandList->ResourceBarrier(m_numBarriersToFlush, m_resourceBarrierBuffer);
		m_numBarriersToFlush = 0;
	}
}


void CommandList::InsertAliasBarrier(GpuResource& before, GpuResource& after, bool flushImmediate)
{
	assert(m_numBarriersToFlush < 16);
	D3D12_RESOURCE_BARRIER& BarrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Aliasing.pResourceBefore = before.GetResource();
	BarrierDesc.Aliasing.pResourceAfter = after.GetResource();

	if (flushImmediate)
	{
		m_commandList->ResourceBarrier(m_numBarriersToFlush, m_resourceBarrierBuffer);
		m_numBarriersToFlush = 0;
	}
}


void CommandList::FlushResourceBarriers()
{
	if (m_numBarriersToFlush == 0)
	{
		return;
	}

	m_commandList->ResourceBarrier(m_numBarriersToFlush, m_resourceBarrierBuffer);
	m_numBarriersToFlush = 0;
}


uint64_t CommandList::Finish(bool wait)
{
	FlushResourceBarriers();

	assert(nullptr != m_currentAllocator);
	ThrowIfFailed(m_commandList->Close());

	auto fenceValue = m_owner->ExecuteCommandList(m_commandList);
	m_owner->DiscardAllocator(fenceValue, m_currentAllocator);
	m_currentAllocator = nullptr;

	m_dynamicDescriptorHeap.CleanupUsedHeaps(fenceValue);

	if (wait)
	{
		m_owner->WaitForFence(fenceValue);
	}

	return fenceValue;
}


void CommandList::Reset()
{
	// We can only call Reset() on previously freed command lists.  The command list persists
	// but we must request a new allocator
	assert(nullptr != m_commandList && nullptr == m_currentAllocator);
	m_currentAllocator = m_owner->RequestAllocator();
	m_commandList->Reset(m_currentAllocator, nullptr);

	m_currentGraphicsRootSignature = nullptr;
	m_currentGraphicsPSO = nullptr;
	m_numBarriersToFlush = 0;

	BindDescriptorHeaps();
}


void CommandList::BindDescriptorHeaps()
{
	uint32_t nonNullHeaps = 0;
	ID3D12DescriptorHeap* heapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	for(uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		ID3D12DescriptorHeap* heapIter = m_currentDescriptorHeaps[i];
		if(heapIter != nullptr)
		{
			heapsToBind[nonNullHeaps++] = heapIter;
		}
	}

	if(nonNullHeaps > 0)
	{
		m_commandList->SetDescriptorHeaps(nonNullHeaps, heapsToBind);
	}
}


CommandList* CommandList::AllocateCommandList()
{
	lock_guard<mutex> lockGuard(s_commandListAllocationMutex);

	CommandList* commandList = nullptr;
	if (s_availableCommandLists.empty())
	{
		commandList = new CommandList;
		s_commandListPool.emplace_back(commandList);
		commandList->Initialize(CommandListManager::GetInstance());
	}
	else
	{
		commandList = s_availableCommandLists.front();
		s_availableCommandLists.pop();
		commandList->Reset();
	}

	assert(nullptr != commandList);

	return commandList;
}


void CommandList::FreeCommandList(CommandList* commandList)
{
	lock_guard<mutex> lockGuard(s_commandListAllocationMutex);

	s_availableCommandLists.push(commandList);
}


void GraphicsCommandList::ClearUAV(ColorBuffer& target)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

	// After binding a UAV, we can get a GPU handle that is required to clear it as a UAV (because it essentially runs
	// a shader to set all of the values).
	D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = m_dynamicDescriptorHeap.UploadDirect(target.GetUAV());
	CD3DX12_RECT ClearRect(0, 0, (LONG)target.GetWidth(), (LONG)target.GetHeight());

	//TODO: My Nvidia card is not clearing UAVs with either Float or Uint variants.
	m_commandList->ClearUnorderedAccessViewFloat(
		GpuVisibleHandle, 
		target.GetUAV(), 
		target.GetResource(), 
		target.GetClearColor(), 
		1, 
		&ClearRect);
}


void GraphicsCommandList::ClearUAV(ColorBuffer& target, const DirectX::XMVECTORF32& clearColor)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

	// After binding a UAV, we can get a GPU handle that is required to clear it as a UAV (because it essentially runs
	// a shader to set all of the values).
	D3D12_GPU_DESCRIPTOR_HANDLE gpuVisibleHandle = m_dynamicDescriptorHeap.UploadDirect(target.GetUAV());
	CD3DX12_RECT clearRect(0, 0, (LONG)target.GetWidth(), (LONG)target.GetHeight());

	//TODO: My Nvidia card is not clearing UAVs with either Float or Uint variants.
	m_commandList->ClearUnorderedAccessViewFloat(
		gpuVisibleHandle,
		target.GetUAV(),
		target.GetResource(),
		clearColor,
		1,
		&clearRect);
}


void GraphicsCommandList::ClearUAV(ColorBuffer& target, const DirectX::XMVECTORU32& clearValue)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

	// After binding a UAV, we can get a GPU handle that is required to clear it as a UAV (because it essentially runs
	// a shader to set all of the values).
	D3D12_GPU_DESCRIPTOR_HANDLE gpuVisibleHandle = m_dynamicDescriptorHeap.UploadDirect(target.GetUAV());
	CD3DX12_RECT clearRect(0, 0, (LONG)target.GetWidth(), (LONG)target.GetHeight());

	//TODO: My Nvidia card is not clearing UAVs with either Float or Uint variants.
	m_commandList->ClearUnorderedAccessViewUint(
		gpuVisibleHandle,
		target.GetUAV(),
		target.GetResource(),
		clearValue.u,
		1,
		&clearRect);
}


void GraphicsCommandList::ClearColor(ColorBuffer& target)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	m_commandList->ClearRenderTargetView(target.GetRTV(), target.GetClearColor(), 0, nullptr);
}


void GraphicsCommandList::ClearColor(ColorBuffer& target, const XMVECTORF32& clearColor)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	m_commandList->ClearRenderTargetView(target.GetRTV(), clearColor, 0, nullptr);
}


void GraphicsCommandList::ClearDepth(DepthBuffer& target)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	m_commandList->ClearDepthStencilView(
		target.GetDSV(), 
		D3D12_CLEAR_FLAG_DEPTH, 
		target.GetClearDepth(), 
		0, 
		0, 
		nullptr);
}


void GraphicsCommandList::ClearDepth(DepthBuffer& target, float clearDepth)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	m_commandList->ClearDepthStencilView(
		target.GetDSV(),
		D3D12_CLEAR_FLAG_DEPTH,
		clearDepth,
		0,
		0,
		nullptr);
}


void GraphicsCommandList::ClearStencil(DepthBuffer& target)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	m_commandList->ClearDepthStencilView(
		target.GetDSV(), 
		D3D12_CLEAR_FLAG_STENCIL, 
		0.0f, 
		target.GetClearStencil(), 
		0, 
		nullptr);
}


void GraphicsCommandList::ClearStencil(DepthBuffer& target, uint32_t clearStencil)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	m_commandList->ClearDepthStencilView(
		target.GetDSV(),
		D3D12_CLEAR_FLAG_STENCIL,
		0.0f,
		clearStencil,
		0,
		nullptr);
}


void GraphicsCommandList::ClearDepthAndStencil(DepthBuffer& target)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	m_commandList->ClearDepthStencilView(
		target.GetDSV(), 
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 
		target.GetClearDepth(), 
		target.GetClearStencil(), 
		0, 
		nullptr);
}


void GraphicsCommandList::ClearDepthAndStencil(DepthBuffer& target, float clearDepth, uint32_t clearStencil)
{
	TransitionResource(target, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	m_commandList->ClearDepthStencilView(
		target.GetDSV(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		clearDepth,
		clearStencil,
		0,
		nullptr);
}


void GraphicsCommandList::SetRenderTargets(uint32_t numRTVs, ColorBuffer* rtvs, DepthBuffer* dsv, bool readOnlyDepth)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8];

	for(uint32_t i = 0; i < numRTVs; ++i)
	{
		TransitionResource(rtvs[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
		rtvHandles[i] = rtvs[i].GetRTV();
	}

	if(dsv)
	{
		if(readOnlyDepth)
		{
			TransitionResource(*dsv, D3D12_RESOURCE_STATE_DEPTH_READ);
			m_commandList->OMSetRenderTargets(numRTVs, rtvHandles, FALSE, &dsv->GetDSVReadOnlyDepth());
		}
		else
		{
			TransitionResource(*dsv, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			m_commandList->OMSetRenderTargets(numRTVs, rtvHandles, FALSE, &dsv->GetDSV());
		}
	}
	else
	{
		m_commandList->OMSetRenderTargets(numRTVs, rtvHandles, FALSE, nullptr);
	}
}


void GraphicsCommandList::SetViewport(const Viewport& vp)
{
	D3D12_VIEWPORT d3dVP = { vp.topLeftX, vp.topLeftY, vp.width, vp.height, vp.minDepth, vp.maxDepth };
	m_commandList->RSSetViewports(1, &d3dVP);
}


void GraphicsCommandList::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
{
	D3D12_VIEWPORT d3dVP = { x, y, w, h, minDepth, maxDepth };
	m_commandList->RSSetViewports(1, &d3dVP);
}


void GraphicsCommandList::SetScissor(const Kodiak::Rectangle& rect)
{
	assert(rect.left < rect.right && rect.top < rect.bottom);
	D3D12_RECT d3dRect = { (LONG)rect.left, (LONG)rect.top, (LONG)rect.right, (LONG)rect.bottom };
	m_commandList->RSSetScissorRects(1, &d3dRect);
}


void GraphicsCommandList::SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
	assert(left < right && top < bottom);
	D3D12_RECT d3dRect = { (LONG)left, (LONG)top, (LONG)right, (LONG)bottom };
	m_commandList->RSSetScissorRects(1, &d3dRect);
}


void GraphicsCommandList::SetViewportAndScissor(const Viewport& vp, const Kodiak::Rectangle& rect)
{
	assert(rect.left < rect.right && rect.top < rect.bottom);
	D3D12_VIEWPORT d3dVP = { vp.topLeftX, vp.topLeftY, vp.width, vp.height, vp.minDepth, vp.maxDepth };
	D3D12_RECT d3dRect = { (LONG)rect.left, (LONG)rect.top, (LONG)rect.right, (LONG)rect.bottom };
	m_commandList->RSSetViewports(1, &d3dVP);
	m_commandList->RSSetScissorRects(1, &d3dRect);
}


void GraphicsCommandList::SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	D3D12_VIEWPORT d3dVP = { (FLOAT)x, (FLOAT)y, (FLOAT)w, (FLOAT)h, 0.0f, 1.0f };
	D3D12_RECT d3dRect = { (LONG)x, (LONG)y, (LONG)(x + w), (LONG)(y + h) };
	m_commandList->RSSetViewports(1, &d3dVP);
	m_commandList->RSSetScissorRects(1, &d3dRect);
}


void GraphicsCommandList::SetPipelineState(const GraphicsPSO& PSO)
{
	auto pipelineState = PSO.GetPipelineStateObject();
	if (pipelineState == m_currentGraphicsPSO)
	{
		return;
	}

	m_commandList->SetPipelineState(pipelineState);
	m_currentGraphicsPSO = pipelineState;
}