#include "Stdafx.h"

#include "CommandList12.h"

#include "RenderTargetView.h"
#include "RenderUtils.h"

using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


void CommandList::Begin()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_allocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_allocator.Get(), nullptr));
}


void CommandList::End()
{
	ThrowIfFailed(m_commandList->Close());
}


void CommandList::SynchronizeRenderTargetViewForRendering(const shared_ptr<RenderTargetView>& rtv)
{
	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv->m_rtv.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
}


void CommandList::SynchronizeRenderTargetViewForPresent(const shared_ptr<RenderTargetView>& rtv)
{
	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv->m_rtv.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}


void CommandList::ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color)
{
	m_commandList->ClearRenderTargetView(rtv->m_rtvHandle, color, 0, nullptr);
}