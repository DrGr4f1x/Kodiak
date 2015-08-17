#include "Stdafx.h"

#include "Renderer.h"

#include "CommandList.h"
#include "DeviceResources.h"
#include "RenderTargetView.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace Concurrency;
using namespace std;


Renderer::Renderer()
{
	m_deviceResources = make_unique<DeviceResources>();

	StartRenderTask();
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
	StopRenderTask();

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


void Renderer::StartRenderTask()
{
	if (m_renderTaskStarted)
	{
		StopRenderTask();
	}

	m_renderTaskEnvironment.stopRenderTask = false;

	m_renderTask = create_task([&]
	{
		// Loop until we get a command to stop the render task, or a command errors out
		while (!m_renderTaskEnvironment.stopRenderTask)
		{
			// Execute a render command, if one is available
			AsyncRenderTask command;
			if (m_renderTaskQueue.try_pop(command))
			{
				command(m_renderTaskEnvironment);
			}
		}
	});

	m_renderTaskStarted = true;
}


void Renderer::StopRenderTask()
{
	m_renderTaskEnvironment.stopRenderTask = true;
	m_renderTask.wait();
	m_renderTaskStarted = false;
}