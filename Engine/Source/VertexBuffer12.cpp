#include "Stdafx.h"

#include "VertexBuffer.h"
#include "VertexBuffer12.h"

#include "CommandList.h"
#include "DeviceResources.h"
#include "RenderEnums.h"


using namespace Kodiak;
using namespace std;


CreateVertexBufferTask::CreateVertexBufferTask(shared_ptr<CommandList> commandList, shared_ptr<VertexBuffer> buffer, unique_ptr<IVertexBufferData> data, Usage usage, const string& name)
	: m_commandList(commandList)
	, m_vertexBuffer(buffer)
	, m_data(move(data))
	, m_usage(usage)
	, m_debugName(name)
{}


void CreateVertexBufferTask::Execute(DeviceResources* deviceResources)
{
	deviceResources->CreateVertexBuffer(m_commandList, m_vertexBuffer, m_data.get(), m_usage, m_debugName);
}


void CreateVertexBufferTask::WaitForGpu()
{
	m_commandList->WaitForGpu();
	m_vertexBuffer->isReady = true;
}