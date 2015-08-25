#pragma once

#include "IAsyncRenderTask.h"

namespace Kodiak
{

// Forward declarations
class DeviceResources;
class IVertexBufferData;
enum class Usage;


class VertexBuffer
{
	friend class DeviceResources;

	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	std::atomic<bool> isReady{ false };
};


class CreateVertexBufferTask : public IAsyncRenderTask
{
public:
	CreateVertexBufferTask(std::shared_ptr<VertexBuffer> buffer, std::unique_ptr<IVertexBufferData> data, Usage usage, const std::string& debugName);

	void Execute(RenderTaskEnvironment& environment);

private:
	std::shared_ptr<VertexBuffer>		m_vertexBuffer;
	std::unique_ptr<IVertexBufferData>	m_data;
	Usage								m_usage;
	std::string							m_debugName;
};

} // namespace Kodiak
