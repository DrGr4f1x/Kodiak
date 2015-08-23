#pragma once

namespace Kodiak
{

// Forward declarations
class CommandList;
class DeviceResources;
class IIndexBufferData;
enum class IndexBufferFormat;
enum class Usage;


class IndexBuffer
{
	friend class DeviceResources;
	friend class CreateIndexBufferTask;

	Microsoft::WRL::ComPtr<ID3D12Resource>	buffer;
	IndexBufferFormat						format;
	D3D12_INDEX_BUFFER_VIEW					indexBufferView;
	std::atomic<bool>						isReady{ false };
};


class CreateIndexBufferTask
{
public:
	CreateIndexBufferTask(
		std::shared_ptr<CommandList> commandList,
		std::shared_ptr<IndexBuffer> buffer, 
		std::unique_ptr<IIndexBufferData> data, 
		Usage usage, 
		const std::string& debugName);

	void Execute(DeviceResources* deviceResources);
	void WaitForGpu();

private:
	std::shared_ptr<CommandList>		m_commandList;
	std::shared_ptr<IndexBuffer>		m_indexBuffer;
	std::unique_ptr<IIndexBufferData>	m_data;
	Usage								m_usage;
	std::string							m_debugName;
};

} // namespace Kodiak
