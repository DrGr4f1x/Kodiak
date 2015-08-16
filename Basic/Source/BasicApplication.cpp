#include "Stdafx.h"

#include "BasicApplication.h"

#include "Engine\Source\CommandList.h"
#include "Engine\Source\Log.h"
#include "Engine\Source\Renderer.h"


using namespace Kodiak;
using namespace std;


BasicApplication::BasicApplication(uint32_t width, uint32_t height, const std::wstring& name)
	: Application(width, height, name)
{}


void BasicApplication::OnInit()
{
	LOG_INFO << "BasicApplication initialize";
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
	m_renderer->Present();
}


void BasicApplication::OnDestroy()
{
	m_renderer->Finalize();
	LOG_INFO << "BasicApplication finalize";
}


bool BasicApplication::OnEvent(MSG msg)
{
	return false;
}


void BasicApplication::PopulateCommandList()
{
	m_commandList->Begin();

	auto rtv = m_renderer->GetBackBuffer();

	m_commandList->ClearRenderTargetView(rtv, DirectX::Colors::CornflowerBlue);

	m_commandList->Present(rtv);

	m_commandList->End();
}