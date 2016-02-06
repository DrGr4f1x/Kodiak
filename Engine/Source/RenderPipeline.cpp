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

#include "ClearColorBufferOp.h"
#include "ClearDepthBufferOp.h"
#include "ColorBuffer.h"
#include "CommandList.h"
#include "DepthBuffer.h"
#include "DeviceManager.h"
#include "Renderer.h"
#include "RenderSceneOp.h"
#include "Scene.h"
#include "SetScissorOp.h"
#include "SetRenderTargetOp.h"
#include "SetViewportOp.h"
#include "UpdateSceneOp.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


Pipeline::~Pipeline()
{
	for (auto operation : m_renderOperations)
	{
		delete operation;
	}
}


void Pipeline::SetName(const string& name)
{
	m_name = name;
}


void Pipeline::ClearColor(shared_ptr<ColorBuffer> colorBuffer)
{
	auto operation = new ClearColorBufferOperation(colorBuffer, colorBuffer->GetClearColor());
	m_renderOperations.push_back(operation);
}


void Pipeline::ClearColor(shared_ptr<ColorBuffer> colorBuffer, const XMVECTORF32& color)
{
	auto operation = new ClearColorBufferOperation(colorBuffer, color);
	m_renderOperations.push_back(operation);
}


void Pipeline::ClearDepth(shared_ptr<DepthBuffer> depthBuffer)
{
	auto operation = new ClearDepthBufferOperation(depthBuffer, depthBuffer->GetClearDepth());
	m_renderOperations.push_back(operation);
}


void Pipeline::SetRenderTarget(std::shared_ptr<ColorBuffer> colorBuffer, std::shared_ptr<DepthBuffer> depthBuffer)
{
	auto operation = new SetRenderTargetOperation(colorBuffer, depthBuffer);
	m_renderOperations.push_back(operation);
}


void Pipeline::SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
{
	auto operation = new SetViewportOperation(topLeftX, topLeftY, width, height, minDepth, maxDepth);
	m_renderOperations.push_back(operation);
}


void Pipeline::SetScissor(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height)
{
	auto operation = new SetScissorOperation(topLeftX, topLeftY, width, height);
	m_renderOperations.push_back(operation);
}


void Pipeline::UpdateScene(shared_ptr<Scene> scene)
{
	auto operation = new UpdateSceneOperation(scene);
	m_renderOperations.push_back(operation);
}


void Pipeline::RenderScene(shared_ptr<Scene> scene)
{
	auto operation = new RenderSceneOperation(scene);
	m_renderOperations.push_back(operation);
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
		operation->PopulateCommandList(commandList);
	}

	commandList.CloseAndExecute();
}