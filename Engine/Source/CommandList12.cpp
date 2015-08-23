#include "Stdafx.h"

#include "CommandList12.h"

#include "DeviceResources12.h"
#include "RenderTargetView12.h"
#include "RenderUtils.h"

using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


CommandList::CommandList(uint32_t frameCount)
	: m_commandLists(frameCount)
	, m_commandAllocators(frameCount)
{
	m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
}


CommandList::~CommandList()
{
	CloseHandle(m_fenceEvent);
}


void CommandList::Begin()
{
	// TODO: Add a fence and a wait to determine when it's safe to reset the command allocator

	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocators[m_currentFrame]->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandLists[m_currentFrame]->Reset(m_commandAllocators[m_currentFrame].Get(), nullptr));
}


void CommandList::End()
{
	ThrowIfFailed(m_commandLists[m_currentFrame]->Close());
}


void CommandList::Present(const std::shared_ptr<RenderTargetView>& rtv)
{
	if (rtv->m_currentResourceState != D3D12_RESOURCE_STATE_PRESENT)
	{
		m_commandLists[m_currentFrame]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv->m_rtv.Get(), rtv->m_currentResourceState, D3D12_RESOURCE_STATE_PRESENT));
		rtv->m_currentResourceState = D3D12_RESOURCE_STATE_PRESENT;
	}
}


void CommandList::ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color)
{
	if (rtv->m_currentResourceState != D3D12_RESOURCE_STATE_RENDER_TARGET)
	{
		m_commandLists[m_currentFrame]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv->m_rtv.Get(), rtv->m_currentResourceState, D3D12_RESOURCE_STATE_RENDER_TARGET));
		rtv->m_currentResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	m_commandLists[m_currentFrame]->ClearRenderTargetView(rtv->m_rtvHandle, color, 0, nullptr);
}


void CommandList::SetCurrentFrame(uint32_t currentFrame)
{
	m_currentFrame = currentFrame;
}


ID3D12GraphicsCommandList* CommandList::GetCommandList()
{
	return m_commandLists[m_currentFrame].Get();
}


void CommandList::WaitForGpu()
{
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
}