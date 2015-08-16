#include "Stdafx.h"

#include "Renderer.h"

#include "CommandList.h"
#include "DeviceResources.h"
#include "RenderTargetView.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


Renderer::Renderer()
{
	m_deviceResources = make_unique<DeviceResources>();
}


void Renderer::SetWindow(uint32_t width, uint32_t height, HWND hwnd)
{
	m_deviceResources->SetWindow(width, height, hwnd);
}


void Renderer::SetWindowSize(uint32_t width, uint32_t height)
{
	m_deviceResources->SetWindowSize(width, height);
}


void Renderer::Finalize()
{
	m_deviceResources->Finalize();
}


shared_ptr<RenderTargetView> Renderer::GetBackBuffer()
{
	return m_deviceResources->GetBackBuffer();
}


void Renderer::Present()
{
	m_deviceResources->Present();
}


shared_ptr<CommandList> Renderer::CreateCommandList()
{
	return m_deviceResources->CreateCommandList();
}


void Renderer::ExecuteCommandList(const shared_ptr<CommandList>& commandList)
{
	m_deviceResources->ExecuteCommandList(commandList);
}