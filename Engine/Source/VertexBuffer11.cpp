#include "Stdafx.h"

#include "VertexBuffer.h"
#include "VertexBuffer11.h"

#include "DeviceResources.h"
#include "Renderer.h"
#include "RenderEnums.h"

using namespace Kodiak;
using namespace std;


CreateVertexBufferTask::CreateVertexBufferTask(shared_ptr<VertexBuffer> buffer, unique_ptr<IVertexBufferData> data, Usage usage, const string& name)
	: m_vertexBuffer(buffer)
	, m_data(move(data))
	, m_usage(usage)
	, m_debugName(name)
{}


void CreateVertexBufferTask::Execute(DeviceResources* deviceResources)
{
	deviceResources->CreateVertexBuffer(m_vertexBuffer, m_data.get(), m_usage, m_debugName);
}