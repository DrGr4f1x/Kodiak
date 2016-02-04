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
#include "Profile.h"
#include "RenderPipeline.h"
#include "RenderUtils.h"
#include "Scene.h"

using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace Concurrency;
using namespace DirectX;
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


class AddStaticModelTask : public IAsyncRenderTask
{
public:
	AddStaticModelTask(shared_ptr<StaticModel> model, shared_ptr<Scene> scene)
		: m_model(model)
		, m_scene(scene)
	{}

	void Execute(RenderTaskEnvironment& environment)
	{
		m_scene->AddStaticModelDeferred(m_model->GetRenderThreadData());
	}

private:
	shared_ptr<StaticModel> m_model;
	shared_ptr<Scene> m_scene;
};


class RemoveStaticModelTask : public IAsyncRenderTask
{
public:
	RemoveStaticModelTask(shared_ptr<StaticModel> model, shared_ptr<Scene> scene)
		: m_model(model)
		, m_scene(scene)
	{}

	void Execute(RenderTaskEnvironment& environment)
	{
		m_scene->RemoveStaticModelDeferred(m_model->GetRenderThreadData());
	}

private:
	shared_ptr<StaticModel> m_model;
	shared_ptr<Scene> m_scene;
};


class UpdateStaticModelMatrixTask : public IAsyncRenderTask
{
public:
	UpdateStaticModelMatrixTask(shared_ptr<RenderThread::StaticModelData> modelData, const XMFLOAT4X4& matrix)
		: m_staticModelData(modelData)
		, m_matrix(matrix)
	{}

	void Execute(RenderTaskEnvironment& environment)
	{
		m_staticModelData->matrix = m_matrix;
		m_staticModelData->isDirty = true;
	}

private:
	shared_ptr<RenderThread::StaticModelData>	m_staticModelData;
	const XMFLOAT4X4							m_matrix;
};


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


void Renderer::EnqueueTask(shared_ptr<IAsyncRenderTask> task)
{
	if (!m_renderTaskEnvironment.stopRenderTask)
	{
		m_renderTaskQueue.push(task);
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


void Renderer::Update()
{
	PROFILE(renderer_Update);

	auto staticModelTask = concurrency::create_task([this] { UpdateStaticModels(); });

	staticModelTask.wait();
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
	auto beginFrame = make_shared<BeginFrameTask>();
	EnqueueTask(beginFrame);

	// Kick off rendering of root pipeline
	auto renderRootPipeline = make_shared<RenderPipelineTask>(m_rootPipeline);
	EnqueueTask(renderRootPipeline);

	// Signal end of frame
	auto endFrame = make_shared<EndFrameTask>(m_rootPipeline->GetPresentSource());
	EnqueueTask(endFrame);
}


void Renderer::AddStaticModelToScene(shared_ptr<StaticModel> model, shared_ptr<Scene> scene)
{
	// Create render thread data if necessary
	auto threadData = model->GetRenderThreadData();
	if (nullptr == threadData)
	{
		model->CreateRenderThreadData();
	}
	threadData = model->GetRenderThreadData();
	
	assert(nullptr != threadData);

	threadData->prepareTask = threadData->prepareTask.then([model, scene]
	{
		auto addStaticModelTask = make_shared<AddStaticModelTask>(model, scene);
		Renderer::GetInstance().EnqueueTask(addStaticModelTask);
	});
}


void Renderer::RemoveStaticModelFromScene(shared_ptr<StaticModel> model, shared_ptr<Scene> scene)
{
	auto threadData = model->GetRenderThreadData();

	assert(nullptr != threadData);

	// Use the prepare task's continuation so we don't try to remove a model that's not fully loaded, and hence
	// hasn't been added to the scene yet.
	threadData->prepareTask = threadData->prepareTask.then([model, scene]
	{
		auto removeStaticModelTask = make_shared<RemoveStaticModelTask>(model, scene);
		Renderer::GetInstance().EnqueueTask(removeStaticModelTask);
	});
}


void Renderer::UpdateStaticModelMatrix(shared_ptr<StaticModel> model)
{
	// Create render thread data if necessary
	auto threadData = model->GetRenderThreadData();
	if (nullptr == threadData)
	{
		model->CreateRenderThreadData();
	}
	threadData = model->GetRenderThreadData();

	assert(nullptr != threadData);

	// Don't need to use the prepareTask continuation here.  Even if the VBs/IBs haven't been created yet,
	// we'll still have a constant buffer to update.
	auto updateStaticModelMatrixTask = make_shared<UpdateStaticModelMatrixTask>(threadData, model->GetMatrix());
	EnqueueTask(updateStaticModelMatrixTask);
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