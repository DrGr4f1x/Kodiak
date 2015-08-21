#include "Stdafx.h"

#include "Renderer.h"

#include "CommandList.h"
#include "DeviceResources.h"
#include "RenderPipeline.h"
#include "RenderTargetView.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace Concurrency;
using namespace std;


Renderer::Renderer()
	: m_rootPipeline(make_shared<RootPipeline>())
{
	m_deviceResources = make_unique<DeviceResources>();

	m_commandListManager = make_unique<CommandListManager>(m_deviceResources.get());

	m_renderTaskEnvironment.deviceResources = m_deviceResources.get();
	m_renderTaskEnvironment.rootPipeline = m_rootPipeline;

	m_rootPipeline->SetName("Root Pipeline");
	m_rootPipeline->SetCommandList(m_commandListManager->CreateCommandList());

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


void Renderer::StartRenderTask()
{
	if (m_renderTaskStarted)
	{
		StopRenderTask();
	}

	m_renderTaskEnvironment.stopRenderTask = false;

	m_renderTask = create_task([&]
	{
		bool endRenderLoop = false;

		// Loop until we're signalled to stop
		while (!endRenderLoop)
		{
			// Process tasks until we've signalled frame complete
			while (!m_renderTaskEnvironment.frameCompleted)
			{
				// Execute a render command, if one is available
				AsyncRenderTask command;
				if (m_renderTaskQueue.try_pop(command))
				{
					command(m_renderTaskEnvironment);
				}
			}

			// Only permit exit after completing the current frame
			endRenderLoop = m_renderTaskEnvironment.stopRenderTask;
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


void Renderer::EnqueueTask(AsyncRenderTask& task)
{
	if (!m_renderTaskEnvironment.stopRenderTask)
	{
		m_renderTaskQueue.push(task);
	}
}


void Renderer::Render()
{
	m_renderTaskEnvironment.frameCompleted = false;

	// Do some pre-render setup
	m_commandListManager->UpdateCommandLists();

	// Kick off rendering of root pipeline
	auto renderRootPipeline(
		[](RenderTaskEnvironment& environment)
		{
			environment.rootPipeline->Execute(environment.deviceResources);
			environment.rootPipeline->Submit(environment.deviceResources);
		}
	);
	AsyncRenderTask renderRootPipelineTask(cref(renderRootPipeline));
	EnqueueTask(renderRootPipelineTask);

	// Signal end of frame
	auto endOfFrame(
		[](RenderTaskEnvironment& environment)
		{
			environment.deviceResources->EndFrame();
			environment.deviceResources->Present();
			environment.frameCompleted = true;
		}
	);
	AsyncRenderTask endOfFrameTask(cref(endOfFrame));
	EnqueueTask(endOfFrameTask);
}


void Renderer::WaitForPreviousFrame()
{
	// Spin while waiting for the previous frame to finish
	while(!m_renderTaskEnvironment.frameCompleted) {}
}