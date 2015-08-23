#include "Stdafx.h"

#include "Renderer.h"

#include "CommandList.h"
#include "DeviceResources.h"
#include "IAsyncRenderTask.h"
#include "IndexBuffer.h"
#include "RenderPipeline.h"
#include "RenderTargetView.h"
#include "RenderUtils.h"
#include "VertexBuffer.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace Concurrency;
using namespace std;


class EndFrameTask : public IAsyncRenderTask
{
public:
	void Execute(RenderTaskEnvironment& environment) override
	{
		environment.deviceResources->EndFrame();
		environment.deviceResources->Present();
		environment.frameCompleted = true;
	}
};


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
				shared_ptr<IAsyncRenderTask> command;
				if (m_renderTaskQueue.try_pop(command))
				{
					command->Execute(m_renderTaskEnvironment);
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


void Renderer::EnqueueTask(shared_ptr<IAsyncRenderTask> task)
{
	if (!m_renderTaskEnvironment.stopRenderTask)
	{
		m_renderTaskQueue.push(task);
	}
}


void Renderer::Render()
{
	// Do some pre-render setup
	m_commandListManager->UpdateCommandLists();

	m_renderTaskEnvironment.frameCompleted = false;

	// Kick off rendering of root pipeline
	auto renderRootPipeline = make_shared<RenderRootPipelineTask>(m_rootPipeline);
	EnqueueTask(renderRootPipeline);


	// Signal end of frame
	auto endFrame = make_shared<EndFrameTask>();
	EnqueueTask(endFrame);
}


void Renderer::WaitForPreviousFrame()
{
	// Spin while waiting for the previous frame to finish
	while(!m_renderTaskEnvironment.frameCompleted) {}
}


shared_ptr<IndexBuffer> Renderer::CreateIndexBuffer(unique_ptr<IIndexBufferData> data, Usage usage, const string& debugName)
{
	auto ibuffer = make_shared<IndexBuffer>();

#if DX12
	auto commandList = m_commandListManager->CreateCommandList();
	auto createIndexBuffer = make_shared<CreateIndexBufferTask>(commandList, ibuffer, move(data), usage, debugName);
	create_task([this, createIndexBuffer]() { createIndexBuffer->Execute(m_deviceResources.get()); }).then([createIndexBuffer]() { createIndexBuffer->WaitForGpu(); });
#elif DX11
	auto createIndexBuffer = make_shared<CreateIndexBufferTask>(ibuffer, move(data), usage, debugName);
	create_task([this, createIndexBuffer]() { createIndexBuffer->Execute(m_deviceResources.get()); });
#endif
	
	return ibuffer;
}


shared_ptr<VertexBuffer> Renderer::CreateVertexBuffer(unique_ptr<IVertexBufferData> data, Usage usage, const string& debugName)
{
	auto vbuffer = make_shared<VertexBuffer>();

#if DX12
	auto commandList = m_commandListManager->CreateCommandList();
	auto createVertexBuffer = make_shared<CreateVertexBufferTask>(commandList, vbuffer, move(data), usage, debugName);
	create_task([this, createVertexBuffer]() { createVertexBuffer->Execute(m_deviceResources.get()); }).then([createVertexBuffer]() { createVertexBuffer->WaitForGpu(); });
#elif DX11
	auto createVertexBuffer = make_shared<CreateVertexBufferTask>(vbuffer, move(data), usage, debugName);
	create_task([this, createVertexBuffer]() { createVertexBuffer->Execute(m_deviceResources.get()); });
#endif
	
	return vbuffer;
}