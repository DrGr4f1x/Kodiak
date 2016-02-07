// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "RenderPipeline.h"

#include "ColorBuffer.h"
#include "CommandList.h"
#include "DepthBuffer.h"
#include "DeviceManager.h"
#include "Renderer.h"
#include "Scene.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


void Pipeline::SetName(const string& name)
{
	m_name = name;
}


void Pipeline::ClearColor(shared_ptr<ColorBuffer> colorBuffer)
{
	m_renderOperations.push_back([colorBuffer](GraphicsCommandList& commandList)
	{
		commandList.ClearColor(*colorBuffer, colorBuffer->GetClearColor());
	});
}


void Pipeline::ClearColor(shared_ptr<ColorBuffer> colorBuffer, const XMVECTORF32& color)
{
	m_renderOperations.push_back([colorBuffer, color](GraphicsCommandList& commandList)
	{
		commandList.ClearColor(*colorBuffer, color);
	});
}


void Pipeline::ClearDepth(shared_ptr<DepthBuffer> depthBuffer)
{
	m_renderOperations.push_back([depthBuffer](GraphicsCommandList& commandList)
	{
		commandList.ClearDepth(*depthBuffer, depthBuffer->GetClearDepth());
	});
}


void Pipeline::SetRenderTarget(std::shared_ptr<ColorBuffer> colorBuffer, std::shared_ptr<DepthBuffer> depthBuffer)
{
	m_renderOperations.push_back([colorBuffer, depthBuffer](GraphicsCommandList& commandList)
	{
		commandList.SetRenderTarget(*colorBuffer, *depthBuffer);
	});
}


void Pipeline::SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
{
	m_renderOperations.push_back([topLeftX, topLeftY, width, height, minDepth, maxDepth](GraphicsCommandList& commandList)
	{
		commandList.SetViewport(topLeftX, topLeftY, width, height, minDepth, maxDepth);
	});
}


void Pipeline::SetScissor(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height)
{
	m_renderOperations.push_back([topLeftX, topLeftY, width, height](GraphicsCommandList& commandList)
	{
		commandList.SetScissor(topLeftX, topLeftY, width, height);
	});
}


void Pipeline::UpdateScene(shared_ptr<Scene> scene)
{
	m_renderOperations.push_back([scene](GraphicsCommandList& commandList)
	{
		scene->Update(commandList);
	});
}


void Pipeline::RenderScene(shared_ptr<Scene> scene)
{
	m_renderOperations.push_back([scene](GraphicsCommandList& commandList)
	{
		scene->Render(commandList);
	});
}


void Pipeline::Present(shared_ptr<ColorBuffer> colorBuffer)
{
	m_presentSource = colorBuffer;
}


shared_ptr<ColorBuffer> Pipeline::GetPresentSource()
{
	return m_presentSource;
}


void Pipeline::Execute()
{
	auto& commandList = GraphicsCommandList::Begin();

	for (auto operation : m_renderOperations)
	{
		operation(commandList);
	}

	commandList.CloseAndExecute();
}