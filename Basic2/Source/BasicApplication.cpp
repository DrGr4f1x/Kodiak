#include "Stdafx.h"

#include "BasicApplication.h"

#include "Engine2\Source\CommandList.h"
#include "Engine2\Source\Renderer.h"


using namespace Kodiak;
using namespace std;


BasicApplication::BasicApplication(uint32_t width, uint32_t height, const std::wstring& name)
	: Application(width, height, name)
{}


void BasicApplication::OnInit()
{
	m_renderer->SetWindow(m_width, m_height, m_hwnd);
	m_commandList = m_renderer->CreateCommandList();
}


void BasicApplication::OnUpdate()
{}


void BasicApplication::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list
	m_renderer->ExecuteCommandList(m_commandList);

	// Present the frame
	const bool bWaitForPreviousFrame = true;
	m_renderer->Present(bWaitForPreviousFrame);
}


void BasicApplication::OnDestroy()
{
	m_renderer->Finalize();
}


bool BasicApplication::OnEvent(MSG msg)
{
	return false;
}


void BasicApplication::PopulateCommandList()
{
	m_commandList->Begin();

	auto rtv = m_renderer->GetBackBuffer();

	m_commandList->SynchronizeRenderTargetViewForRendering(rtv);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtv, clearColor);

	m_commandList->SynchronizeRenderTargetViewForPresent(rtv);

	m_commandList->End();
}