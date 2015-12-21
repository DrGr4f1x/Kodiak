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
#include "IAsyncRenderTask.h"
#include "IndexBuffer.h"
#include "Model.h"
#include "RenderPipeline.h"
#include "RenderUtils.h"
#include "Scene.h"

#include <concurrent_queue.h>


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace Concurrency;
using namespace std;


class BeginFrameTask : public IAsyncRenderTask
{
public:
	void Execute(RenderTaskEnvironment& environment) override
	{
		environment.deviceManager->BeginFrame();
	}
};

class EndFrameTask : public IAsyncRenderTask
{
public:
	EndFrameTask(shared_ptr<ColorBuffer> presentSource)
		: m_presentSource(presentSource)
	{}

	void Execute(RenderTaskEnvironment& environment) override
	{
		environment.deviceManager->Present(m_presentSource);
		environment.currentFrame += 1;
		environment.frameCompleted = true;
	}

private:
	shared_ptr<ColorBuffer> m_presentSource;
};


class AddModelTask : public IAsyncRenderTask
{
public:
	AddModelTask(shared_ptr<Scene> scene, shared_ptr<Model> model)
		: m_scene(scene)
		, m_model(model)
	{}

	void Execute(RenderTaskEnvironment& environment) override
	{
		m_scene->AddModel(m_model);
	}

private:
	shared_ptr<Scene> m_scene;
	shared_ptr<Model> m_model;
};


class UpdateModelTransformTask : public IAsyncRenderTask
{
public:
	UpdateModelTransformTask(shared_ptr<Model> model, const DirectX::XMFLOAT4X4& matrix)
		: m_model(model)
		, m_matrix(matrix)
	{}

	void Execute(RenderTaskEnvironment& environment) override
	{
		m_model->SetTransform(m_matrix);
	}

private:
	shared_ptr<Model>		m_model;
	DirectX::XMFLOAT4X4		m_matrix;
};


namespace Renderer
{

std::shared_ptr<Pipeline>			g_rootPipeline{ nullptr };
std::unique_ptr<DeviceManager>		g_deviceManager{ nullptr };
RenderTaskEnvironment				g_renderTaskEnvironment;
bool								g_renderTaskStarted{ false };
Concurrency::task<void>				g_renderTask;
Concurrency::concurrent_queue<std::shared_ptr<IAsyncRenderTask>>	g_renderTaskQueue;


void StartRenderTask();
void StopRenderTask();



void Initialize()
{
	g_rootPipeline = make_shared<Pipeline>();
	g_deviceManager = make_unique<DeviceManager>();

	g_renderTaskEnvironment.deviceManager = g_deviceManager.get();
	g_renderTaskEnvironment.rootPipeline = g_rootPipeline;

	g_rootPipeline->SetName("Root Pipeline");

	StartRenderTask();
}


void Finalize()
{
	StopRenderTask();

	g_deviceManager->Finalize();
}


void SetWindow(uint32_t width, uint32_t height, HWND hwnd)
{
	g_deviceManager->SetWindow(width, height, hwnd);
}


void SetWindowSize(uint32_t width, uint32_t height)
{
	g_deviceManager->SetWindowSize(width, height);
}


void StartRenderTask()
{
	if (g_renderTaskStarted)
	{
		StopRenderTask();
	}

	g_renderTaskEnvironment.stopRenderTask = false;

	g_renderTask = create_task([&]
	{
		bool endRenderLoop = false;

		// Loop until we're signalled to stop
		while (!endRenderLoop)
		{
			// Process tasks until we've signalled frame complete
			while (!g_renderTaskEnvironment.frameCompleted)
			{
				// Execute a render command, if one is available
				shared_ptr<IAsyncRenderTask> command;
				if (g_renderTaskQueue.try_pop(command))
				{
					command->Execute(g_renderTaskEnvironment);
				}
			}

			// Only permit exit after completing the current frame
			endRenderLoop = g_renderTaskEnvironment.stopRenderTask;
		}
	});

	g_renderTaskStarted = true;
}


void StopRenderTask()
{
	g_renderTaskEnvironment.stopRenderTask = true;
	g_renderTask.wait();
	g_renderTaskStarted = false;
}


void Render()
{
	// Wait on the previous frame
	while (!g_renderTaskEnvironment.frameCompleted)
	{
		this_thread::yield();
	}

	g_renderTaskEnvironment.frameCompleted = false;

	// Start new frame
	auto beginFrame = make_shared<BeginFrameTask>();
	EnqueueTask(beginFrame);

	// Kick off rendering of root pipeline
	auto renderRootPipeline = make_shared<RenderPipelineTask>(g_rootPipeline);
	EnqueueTask(renderRootPipeline);

	// Signal end of frame
	auto endFrame = make_shared<EndFrameTask>(g_rootPipeline->GetPresentSource());
	EnqueueTask(endFrame);
}


shared_ptr<Pipeline> GetRootPipeline()
{
	return g_rootPipeline;
}


void EnqueueTask(shared_ptr<IAsyncRenderTask> task)
{
	if (!g_renderTaskEnvironment.stopRenderTask)
	{
		g_renderTaskQueue.push(task);
	}
}


void AddModel(shared_ptr<Scene> scene, shared_ptr<Model> model)
{
	auto addModelTask = make_shared<AddModelTask>(scene, model);
	EnqueueTask(addModelTask);
}


void UpdateModelTransform(shared_ptr<Model> model, const DirectX::XMFLOAT4X4& matrix)
{
	auto updateTask = make_shared<UpdateModelTransformTask>(model, matrix);
	EnqueueTask(updateTask);
}

} // namespace Renderer


namespace Kodiak
{

shared_ptr<ColorBuffer> CreateColorBuffer(const std::string& name, uint32_t width, uint32_t height, uint32_t arraySize, ColorFormat format,
	const DirectX::XMVECTORF32& clearColor)
{
	auto colorBuffer = make_shared<ColorBuffer>(clearColor);

	colorBuffer->Create(Renderer::g_deviceManager.get(), name, width, height, arraySize, format);

	return colorBuffer;
}


shared_ptr<DepthBuffer> CreateDepthBuffer(const std::string& name, uint32_t width, uint32_t height, DepthFormat format, float clearDepth,
	uint32_t clearStencil)
{
	auto depthBuffer = make_shared<DepthBuffer>(clearDepth, clearStencil);

	depthBuffer->Create(Renderer::g_deviceManager.get(), name, width, height, format);

	return depthBuffer;
}

} // namespace Kodiak