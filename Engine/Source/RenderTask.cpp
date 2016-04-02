// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "RenderTask.h"

#include "ColorBuffer.h"
#include "CommandList.h"
#include "DepthBuffer.h"
#include "DeviceManager.h"
#include "Renderer.h"
#include "Scene.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


void RenderTask::SetName(const string& name)
{
	m_name = name;
}


void RenderTask::ClearColor(shared_ptr<ColorBuffer> colorBuffer)
{
	m_renderSteps.push_back([colorBuffer](GraphicsCommandList* commandList)
	{
		commandList->ClearColor(*colorBuffer, colorBuffer->GetClearColor());
	});
}


void RenderTask::ClearColor(shared_ptr<ColorBuffer> colorBuffer, const XMVECTORF32& color)
{
	m_renderSteps.push_back([colorBuffer, color](GraphicsCommandList* commandList)
	{
		commandList->ClearColor(*colorBuffer, color);
	});
}


void RenderTask::ClearDepth(shared_ptr<DepthBuffer> depthBuffer)
{
	m_renderSteps.push_back([depthBuffer](GraphicsCommandList* commandList)
	{
		commandList->ClearDepth(*depthBuffer, depthBuffer->GetClearDepth());
	});
}


void RenderTask::SetRenderTarget(std::shared_ptr<ColorBuffer> colorBuffer, std::shared_ptr<DepthBuffer> depthBuffer)
{
	m_renderSteps.push_back([colorBuffer, depthBuffer](GraphicsCommandList* commandList)
	{
		commandList->SetRenderTarget(*colorBuffer, *depthBuffer);
	});
}


void RenderTask::SetDepthStencilTarget(shared_ptr<DepthBuffer> depthBuffer)
{
	m_renderSteps.push_back([depthBuffer](GraphicsCommandList* commandList)
	{
		commandList->SetDepthStencilTarget(*depthBuffer);
	});
}


void RenderTask::SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
{
	m_renderSteps.push_back([topLeftX, topLeftY, width, height, minDepth, maxDepth](GraphicsCommandList* commandList)
	{
		commandList->SetViewport(topLeftX, topLeftY, width, height, minDepth, maxDepth);
	});
}


void RenderTask::SetScissor(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height)
{
	m_renderSteps.push_back([topLeftX, topLeftY, width, height](GraphicsCommandList* commandList)
	{
		commandList->SetScissor(topLeftX, topLeftY, width, height);
	});
}


void RenderTask::UpdateScene(shared_ptr<Scene> scene)
{
	m_renderSteps.push_back([scene](GraphicsCommandList* commandList)
	{
		scene->Update(commandList);
	});
}


void RenderTask::RenderScenePass(shared_ptr<RenderPass> renderPass, shared_ptr<Scene> scene)
{
	m_renderSteps.push_back([renderPass, scene](GraphicsCommandList* commandList)
	{
		scene->Render(renderPass, commandList);
	});
}


void RenderTask::Continue(shared_ptr<RenderTask> antecedent)
{
	m_antecedents.push_back(antecedent);
	antecedent->m_predecessors.push_back(shared_from_this());
}


void RenderTask::Start(Concurrency::task<void>& currentTask)
{
	assert(!m_predecessors.empty());

	// If there's only one preceding task, run as a continuation of its task
	if (m_predecessors.size() == 1)
	{
		currentTask = currentTask.then([this] { Run(); });

		for (auto antecedent : m_antecedents)
		{
			antecedent->Start(currentTask);
		}
	}
	// There are multiple preceding tasks, so we must wait on them to complete before continuing
	else
	{
		m_predecessorTasks.push_back(currentTask);

		// We have all the tasks necessary to wait-then-continue
		if (m_predecessorTasks.size() == m_predecessors.size())
		{
			currentTask = Concurrency::when_all(begin(m_predecessorTasks), end(m_predecessorTasks)).then([this] { Run(); });

			for (auto antecedent : m_antecedents)
			{
				antecedent->Start(currentTask);
			}

			m_predecessorTasks.clear();
		}
	}
}


void RenderTask::Run()
{
	auto commandList = GraphicsCommandList::Begin();

	for (const auto& operation : m_renderSteps)
	{
		operation(commandList);
	}

	commandList->CloseAndExecute(true);
	commandList = nullptr;
}


void RootRenderTask::Start()
{
	m_rootTask = concurrency::create_task([this] { Run(); });

	for (auto antecendent : m_antecedents)
	{
		antecendent->Start(m_rootTask);
	}
}


void RootRenderTask::Wait()
{
	m_rootTask.wait();
}


void RootRenderTask::Present(shared_ptr<ColorBuffer> colorBuffer)
{
	m_presentSource = colorBuffer;
}


shared_ptr<ColorBuffer> RootRenderTask::GetPresentSource()
{
	return m_presentSource;
}