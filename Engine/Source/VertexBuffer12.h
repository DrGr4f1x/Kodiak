#pragma once

namespace Kodiak
{
	
// Forward declarations
class CommandList;
class DeviceResources;
class IVertexBufferData;
enum class Usage;


class VertexBuffer
{
	friend class DeviceResources;
	friend class CreateVertexBufferTask;

	Microsoft::WRL::ComPtr<ID3D12Resource>	buffer;
	D3D12_VERTEX_BUFFER_VIEW				vertexBufferView;
	std::atomic<bool>						isReady{ false };
};


class CreateVertexBufferTask
{
public:
	CreateVertexBufferTask(
		std::shared_ptr<CommandList> commandList, 
		std::shared_ptr<VertexBuffer> buffer, 
		std::unique_ptr<IVertexBufferData> data, 
		Usage usage, 
		const std::string& debugName);

	void Execute(DeviceResources* deviceResources);
	void WaitForGpu();

private:
	std::shared_ptr<CommandList>		m_commandList;
	std::shared_ptr<VertexBuffer>		m_vertexBuffer;
	std::unique_ptr<IVertexBufferData>	m_data;
	Usage								m_usage;
	std::string							m_debugName;
};

} // namespace Kodiak
