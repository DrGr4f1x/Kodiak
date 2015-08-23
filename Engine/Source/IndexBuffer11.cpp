#include "Stdafx.h"

#include "IndexBuffer.h"
#include "IndexBuffer11.h"

#include "DeviceResources.h"
#include "RenderEnums.h"


using namespace Kodiak;
using namespace std;


CreateIndexBufferTask::CreateIndexBufferTask(shared_ptr<IndexBuffer> buffer, unique_ptr<IIndexBufferData> data, Usage usage, const string& name)
	: m_indexBuffer(buffer)
	, m_data(move(data))
	, m_usage(usage)
	, m_debugName(name)
{}


void CreateIndexBufferTask::Execute(DeviceResources* deviceResources)
{
	deviceResources->CreateIndexBuffer(m_indexBuffer, m_data.get(), m_usage, m_debugName);
}