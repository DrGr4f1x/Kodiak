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
#include "RenderTask.h"
#include "RenderUtils.h"
#include "ResourceLoader.h"
#include "SamplerManager.h"
#include "Scene.h"
#include "TextureResource.h"

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
	m_renderTaskEnvironment.rootTask = nullptr;

	SamplerManager::GetInstance().Initialize();

	StartRenderTask();
}


void Renderer::Finalize()
{
	StopRenderTask();

	m_renderTaskEnvironment.rootTask = nullptr;

	SamplerManager::GetInstance().Shutdown();

	DeviceManager::GetInstance().Finalize();
}


void Renderer::EnqueueTask(std::function<void(RenderTaskEnvironment&)> callback)
{
	if (!m_renderTaskEnvironment.stopRenderTask)
	{
		m_renderTaskQueue.push(callback);
	}
}


void Renderer::Update()
{
	ResourceLoader<TextureResource>::GetInstance().Update();
}


void Renderer::Render(shared_ptr<RootRenderTask> rootTask, bool bHDRPresent, const PresentParameters& params)
{
	PROFILE_BEGIN(itt_render);

	// Wait on the previous frame
	while (!m_renderTaskEnvironment.frameCompleted)
	{
		this_thread::yield();
	}

	m_renderTaskEnvironment.frameCompleted = false;
	m_renderTaskEnvironment.rootTask = rootTask;

	// Start new frame
	EnqueueTask([](RenderTaskEnvironment& rte) 
	{ 
		DeviceManager::GetInstance().BeginFrame(); 
	});
	
	// Kick off rendering of root pipeline
	EnqueueTask([](RenderTaskEnvironment& rte) { rte.rootTask->Start(); });

	// Signal end of frame
	EnqueueTask([bHDRPresent, params](RenderTaskEnvironment& rte)
	{
		rte.rootTask->Wait();

		DeviceManager::GetInstance().Present(rte.rootTask->GetPresentSource(), bHDRPresent, params);

		rte.currentFrame += 1;
		rte.frameCompleted = true;
	});

	PROFILE_END();
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
		SetThreadRole(ThreadRole::RenderMain);

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
	
}