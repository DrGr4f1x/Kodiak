#include "Stdafx.h"

#include "CommandList11.h"

#include "RenderTargetView.h"

using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


void CommandList::Begin()
{
	assert(!m_bIsRecording);
	m_bIsRecording = true;
}


void CommandList::End()
{
	assert(m_bIsRecording);
	m_bIsRecording = false;

	m_deferredContext->FinishCommandList(TRUE, &m_commandList);
}


void CommandList::ClearRenderTargetView(const shared_ptr<RenderTargetView>& rtv, const float* clearColor)
{
	assert(m_bIsRecording);
	m_deferredContext->ClearRenderTargetView(rtv->m_rtv.Get(), clearColor);
}