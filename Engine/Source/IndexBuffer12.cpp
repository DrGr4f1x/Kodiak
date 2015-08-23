#include "Stdafx.h"

#include "IndexBuffer.h"
#include "IndexBuffer12.h"

#include "CommandList.h"
#include "DeviceResources.h"
#include "RenderEnums.h"


using namespace Kodiak;
using namespace std;


CreateIndexBufferTask::CreateIndexBufferTask(shared_ptr<CommandList> commandList, shared_ptr<IndexBuffer> buffer, unique_ptr<IIndexBufferData> data, Usage usage, const string& name)
	: m_commandList(commandList)
	, m_indexBuffer(buffer)
	, m_data(move(data))
	, m_usage(usage)
	, m_debugName(name)
{}


void CreateIndexBufferTask::Execute(DeviceResources* deviceResources)
{
	deviceResources->CreateIndexBuffer(m_commandList, m_indexBuffer, m_data.get(), m_usage, m_debugName);
}


void CreateIndexBufferTask::WaitForGpu()
{
	m_commandList->WaitForGpu();
	m_indexBuffer->isReady = true;
}