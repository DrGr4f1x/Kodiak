#include "Stdafx.h"

#include "RenderPipeline.h"

#include "ClearRenderTargetOp.h"
#include "CommandList.h"
#include "DeviceResources.h"
#include "PresentRenderTargetOp.h"
#include "Renderer.h"
#include "RenderTargetView.h"


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


void Pipeline::SetCommandList(const shared_ptr<CommandList>& commandList)
{
	m_commandList = commandList;
}


void Pipeline::Begin()
{
	m_commandList->Begin();
}


void Pipeline::End()
{
	m_commandList->End();
}


void Pipeline::ClearRenderTargetView(const shared_ptr<RenderTargetView>& rtv, const XMVECTORF32& color)
{
	auto operation = new ClearRenderTargetOperation(rtv, color);
	m_renderOperations.push_back(operation);
}


void Pipeline::Execute(DeviceResources* deviceResources)
{
	m_commandList->Begin();

	for (auto operation : m_renderOperations)
	{
		operation->PopulateCommandList(m_commandList);
	}

	m_commandList->End();
}


void Pipeline::Submit(DeviceResources* deviceResources)
{
	deviceResources->ExecuteCommandList(m_commandList);
}


void RootPipeline::Present(const std::shared_ptr<RenderTargetView>& rtv)
{
	auto operation = new PresentRenderTargetOperation(rtv);
	m_renderOperations.push_back(operation);
}


RenderRootPipelineTask::RenderRootPipelineTask(shared_ptr<RootPipeline> pipeline)
	: m_pipeline(pipeline)
{}


void RenderRootPipelineTask::Execute(RenderTaskEnvironment& environment)
{
	m_pipeline->Execute(environment.deviceResources);
	m_pipeline->Submit(environment.deviceResources);
}