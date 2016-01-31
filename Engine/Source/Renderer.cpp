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


namespace Kodiak
{

class AddStaticModelAction
{
public:
	AddStaticModelAction(shared_ptr<StaticModel> model, shared_ptr<Scene> scene) : m_model(model), m_scene(scene) {}

	void Execute()
	{
		if (!m_model->m_renderThreadData)
		{
			m_model->CreateRenderThreadData();
		}

		// Need to pass local variables to the continuation lambda, so the lambda will copy-by-value
		// and keep the smart pointer references alive.
		auto model = m_model;
		auto scene = m_scene;
		m_model->m_renderThreadData->prepareTask.then([model, scene] { scene->AddStaticModelDeferred(model->m_renderThreadData); });
	}

private:
	shared_ptr<StaticModel>		m_model;
	shared_ptr<Scene>			m_scene;
};

class RemoveStaticModelAction
{
public:
	RemoveStaticModelAction(shared_ptr<StaticModel> model, shared_ptr<Scene> scene) : m_model(model), m_scene(scene) {}

	void Execute()
	{
		m_scene->RemoveStaticModelDeferred(m_model->m_renderThreadData);
	}

private:
	shared_ptr<StaticModel>		m_model;
	shared_ptr<Scene>			m_scene;
};

}


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


void Renderer::AddStaticModelToScene(std::shared_ptr<StaticModel> model, std::shared_ptr<Scene> scene)
{
	if (model->m_renderThreadData)
	{
		model->m_renderThreadData->prepareTask.then([model, scene] {scene->AddStaticModelDeferred(model->m_renderThreadData); });
	}
	else
	{
		auto meshAction = make_shared<AddStaticModelAction>(model, scene);
		m_pendingStaticModelAdds.push(meshAction);
	}
}


void Renderer::RemoveStaticModelFromScene(std::shared_ptr<StaticModel> model, std::shared_ptr<Scene> scene)
{
	auto meshAction = make_shared<RemoveStaticModelAction>(model, scene);
	m_pendingStaticModelRemovals.push(meshAction);
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

	auto task = concurrency::create_task([this]
	{
		size_t numActions = 0;
		shared_ptr<RemoveStaticModelAction> action;

		while (numActions++ < m_numStaticModelRemovals && m_pendingStaticModelRemovals.try_pop(action))
		{
			action->Execute();
		}
	});

	task = task.then([this]
	{
		size_t numActions = 0;
		shared_ptr<AddStaticModelAction> action;

		while (numActions++ < m_numStaticModelAdds && m_pendingStaticModelAdds.try_pop(action))
		{
			action->Execute();
		}
	});

	task.wait();
}


namespace Kodiak
{

namespace RenderThread
{

void AddModel(shared_ptr<Scene> scene, shared_ptr<Model> model)
{
	auto addModelTask = make_shared<AddModelTask>(scene, model);
	Renderer::GetInstance().EnqueueTask(addModelTask);
}


void UpdateModelTransform(shared_ptr<Model> model, const DirectX::XMFLOAT4X4& matrix)
{
	auto updateTask = make_shared<UpdateModelTransformTask>(model, matrix);
	Renderer::GetInstance().EnqueueTask(updateTask);
}

} // namespace RenderThread


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