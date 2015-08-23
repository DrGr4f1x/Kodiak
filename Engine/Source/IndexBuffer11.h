#pragma once

namespace Kodiak
{

// Forward declarations
class DeviceResources;
class IIndexBufferData;
enum class IndexBufferFormat;
enum class Usage;


class IndexBuffer
{
	friend class DeviceResources;

	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	IndexBufferFormat format;
	std::atomic<bool> isReady{ false };
};


class CreateIndexBufferTask
{
public:
	CreateIndexBufferTask(std::shared_ptr<IndexBuffer> buffer, std::unique_ptr<IIndexBufferData> data, Usage usage, const std::string& debugName);

	void Execute(DeviceResources* deviceResources);

private:
	std::shared_ptr<IndexBuffer>		m_indexBuffer;
	std::unique_ptr<IIndexBufferData>	m_data;
	Usage								m_usage;
	std::string							m_debugName;
};

} // namespace Kodiak