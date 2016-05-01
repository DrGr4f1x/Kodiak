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
#include "ComputeKernel.h"
#include "DepthBuffer.h"
#include "DeviceManager.h"
#include "RenderEnums.h"
#include "Renderer.h"
#include "Scene.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


void RenderTask::SetName(const string& name)
{
	m_name = name;
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
		currentTask = currentTask.then([this] { Render(); });

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
			currentTask = Concurrency::when_all(begin(m_predecessorTasks), end(m_predecessorTasks)).then([this] { Render(); });

			for (auto antecedent : m_antecedents)
			{
				antecedent->Start(currentTask);
			}

			m_predecessorTasks.clear();
		}
	}
}


void RootRenderTask::Start()
{
	m_rootTask = concurrency::create_task([this] { Render(); });

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