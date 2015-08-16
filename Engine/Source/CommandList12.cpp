#include "Stdafx.h"

#include "CommandList12.h"

#include "DeviceResources12.h"
#include "RenderTargetView12.h"
#include "RenderUtils.h"

using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


void CommandList::Begin()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_deviceResources->GetCommandAllocator()->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_deviceResources->GetCommandAllocator().Get(), nullptr));
}


void CommandList::End()
{
	ThrowIfFailed(m_commandList->Close());
}


void CommandList::Present(const std::shared_ptr<RenderTargetView>& rtv)
{
	if (rtv->m_currentResourceState != D3D12_RESOURCE_STATE_PRESENT)
	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv->m_rtv.Get(), rtv->m_currentResourceState, D3D12_RESOURCE_STATE_PRESENT));
		rtv->m_currentResourceState = D3D12_RESOURCE_STATE_PRESENT;
	}
}


void CommandList::ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color)
{
	if (rtv->m_currentResourceState != D3D12_RESOURCE_STATE_RENDER_TARGET)
	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv->m_rtv.Get(), rtv->m_currentResourceState, D3D12_RESOURCE_STATE_RENDER_TARGET));
		rtv->m_currentResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	m_commandList->ClearRenderTargetView(rtv->m_rtvHandle, color, 0, nullptr);
}