#include "Stdafx.h"

#include "BasicApplication.h"

#include "Engine\Source\CommandList.h"
#include "Engine\Source\Log.h"
#include "Engine\Source\Renderer.h"
#include "Engine\Source\RenderPipeline.h"


using namespace Kodiak;
using namespace std;


BasicApplication::BasicApplication(uint32_t width, uint32_t height, const std::wstring& name)
	: Application(width, height, name)
{}


void BasicApplication::OnInit()
{
	LOG_INFO << "BasicApplication initialize";
	m_renderer->SetWindow(m_width, m_height, m_hwnd);
	
	// Setup the root rendering pipeline
	auto pipeline = m_renderer->GetRootPipeline();
	auto rtv = m_renderer->GetBackBuffer();

	pipeline->ClearRenderTargetView(rtv, DirectX::Colors::CornflowerBlue);
	pipeline->Present(rtv);
}


void BasicApplication::OnUpdate()
{}


void BasicApplication::OnDestroy()
{
	m_renderer->Finalize();
	LOG_INFO << "BasicApplication finalize";
}


bool BasicApplication::OnEvent(MSG msg)
{
	return false;
}