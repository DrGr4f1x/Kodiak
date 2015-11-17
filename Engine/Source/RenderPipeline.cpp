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
#include "ColorBuffer.h"
#include "CommandList.h"
#include "DeviceManager.h"
#include "Renderer.h"
#include "RenderSceneOp.h"
#include "Scene.h"


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


RenderPipelineTask::RenderPipelineTask(shared_ptr<Pipeline> pipeline)
	: m_pipeline(pipeline)
{}


void RenderPipelineTask::Execute(RenderTaskEnvironment& environment)
{
	m_pipeline->Execute();
}