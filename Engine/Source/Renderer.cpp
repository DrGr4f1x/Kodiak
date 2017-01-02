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


namespace Kodiak
{

void EnqueueRenderCommand(function<void()>&& command)
{
	auto& renderer = Renderer::GetInstance();

	if (renderer.IsRenderThreadRunning())
	{
		renderer.EnqueueRenderCommand(move(command));
	}
	else
	{
		assert(GetThreadRole() == ThreadRole::Main);
		command();
	}
}

} // Kodiak namespace


namespace
{
atomic_bool g_terminateRenderThread{ false };
} // anonymous namespace


Renderer& Renderer::GetInstance()
{
	static Renderer instance;
	return instance;
}


void Renderer::Initialize()
{
	SamplerManager::GetInstance().Initialize();
}


void Renderer::Finalize()
{
	if (m_renderThreadRunning)
	{
		StopRenderThread();
	}
	
	SamplerManager::GetInstance().Shutdown();

	DeviceManager::GetInstance().Finalize();
}


void Renderer::EnableRenderThread(bool enable)
{
	assert(GetThreadRole() == ThreadRole::Main);

	m_renderThreadStateChanged = (enable != m_renderThreadRunning);
}


void Renderer::ToggleRenderThread()
{
	EnableRenderThread(!m_renderThreadRunning);
}


bool Renderer::IsRenderThreadRunning() const
{
	return m_renderThreadRunning;
}


void Renderer::EnqueueRenderCommand(function<void()>&& command)
{
	m_renderCommandQueue.push(move(command));
}


void Renderer::Update()
{
	ResourceLoader::GetInstance().Update();
}


void Renderer::Render()
{
	// Start or stop the render thread if needed
	if (m_renderThreadStateChanged)
	{
		if (m_renderThreadRunning)
		{
			StopRenderThread();
		}
		else
		{
			StartRenderThread();
		}
		m_renderThreadStateChanged = false;
	}

	if (m_renderThreadRunning)
	{
		// TODO: Allow multiple frames in flight
		atomic_bool frameComplete{ false };
		EnqueueRenderCommand([&frameComplete]() { frameComplete = true; });
		while (!frameComplete)
		{
			this_thread::yield();
		}
	}
}


void Renderer::StartRenderThread()
{
	assert(!m_renderThreadRunning);

	g_terminateRenderThread = false;

	m_renderThreadFuture = async(launch::async, [this]()
	{
		while (!g_terminateRenderThread)
		{
			function<void()> command;
			if (this->m_renderCommandQueue.try_pop(command))
			{
				command();
			}
		}
		this->m_renderCommandQueue.clear();
	});

	m_renderThreadRunning = true;
}


void Renderer::StopRenderThread()
{
	assert(m_renderThreadRunning);
	assert(m_renderThreadFuture.valid());

	g_terminateRenderThread = true;
	m_renderThreadFuture.get();
	m_renderThreadRunning = false;
}