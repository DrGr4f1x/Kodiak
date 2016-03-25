// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Renderer.h"

#include "ColorBuffer.h"
#include "CommandList.h"
#include "CommandListManager.h"
#include "DepthBuffer.h"
#include "DeviceManager.h"
#include "Format.h"
#include "IndexBuffer.h"
#include "Model.h"
#include "Profile.h"
#include "RenderPipeline.h"
#include "RenderUtils.h"
#include "Scene.h"

using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace Concurrency;
using namespace DirectX;
using namespace std;


Renderer& Renderer::GetInstance()
{
	static Renderer instance;
	return instance;
}


void Renderer::Initialize()
{
	m_rootPipeline = make_shared<Pipeline>();
	m_deviceManager = make_unique<DeviceManager>();

	m_renderTaskEnvironment.deviceManager = m_deviceManager.get();
	m_renderTaskEnvironment.rootPipeline = m_rootPipeline;

	m_rootPipeline->SetName("Root Pipeline");

	StartRenderTask();
}


void Renderer::Finalize()
{
	StopRenderTask();

	m_deviceManager->Finalize();
}


void Renderer::EnqueueTask(std::function<void(RenderTaskEnvironment&)> callback)
{
	if (!m_renderTaskEnvironment.stopRenderTask)
	{
		m_renderTaskQueue.push(callback);
	}
}


void Renderer::SetWindow(uint32_t width, uint32_t height, HWND hwnd)
{
	m_deviceManager->SetWindow(width, height, hwnd);
}


void Renderer::SetWindowSize(uint32_t width, uint32_t height)
{
	m_deviceManager->SetWindowSize(width, height);
}


void Renderer::Render()
{
	PROFILE(renderer_Render);

	// Wait on the previous frame
	while (!m_renderTaskEnvironment.frameCompleted)
	{
		this_thread::yield();
	}

	m_renderTaskEnvironment.frameCompleted = false;

	// Start new frame
	EnqueueTask([](RenderTaskEnvironment& rte) { rte.deviceManager->BeginFrame(); });
	
	// Kick off rendering of root pipeline
	EnqueueTask([](RenderTaskEnvironment& rte) { rte.rootPipeline->Execute(); });

	// Signal end of frame
	auto presentSource = m_rootPipeline->GetPresentSource();
	EnqueueTask([presentSource](RenderTaskEnvironment& rte)
	{
		rte.deviceManager->Present(presentSource);
		rte.currentFrame += 1;
		rte.frameCompleted = true;
	});
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
				std::function<void(RenderTaskEnvironment&)> command;
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


void Renderer::UpdateStaticModels()
{
	PROFILE(renderer_UpdateStaticModels);

	
}


namespace Kodiak
{

shared_ptr<ColorBuffer> CreateColorBuffer(const std::string& name, uint32_t width, uint32_t height, uint32_t arraySize, ColorFormat format,
	const DirectX::XMVECTORF32& clearColor)
{
	auto colorBuffer = make_shared<ColorBuffer>(clearColor);

	colorBuffer->Create(Renderer::GetInstance().GetDeviceManager(), name, width, height, arraySize, format);

	return colorBuffer;
}


shared_ptr<DepthBuffer> CreateDepthBuffer(const std::string& name, uint32_t width, uint32_t height, DepthFormat format, float clearDepth,
	uint32_t clearStencil)
{
	auto depthBuffer = make_shared<DepthBuffer>(clearDepth, clearStencil);

	depthBuffer->Create(Renderer::GetInstance().GetDeviceManager(), name, width, height, format);

	return depthBuffer;
}

} // namespace Kodiak