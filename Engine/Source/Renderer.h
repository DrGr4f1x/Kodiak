#pragma once

#include <concurrent_queue.h>

namespace Kodiak
{

// Forward declarations
class CommandList;
class CommandListManager;
class DeviceResources;
class IAsyncRenderTask;
class IIndexBufferData;
class IVertexBufferData;
class IndexBuffer;
class RenderTargetView;
class RootPipeline;
class VertexBuffer;

enum class Usage;

struct RenderTaskEnvironment 
{
	DeviceResources* deviceResources;
	std::shared_ptr<RootPipeline> rootPipeline;
	std::atomic<bool> stopRenderTask{ false };
	std::atomic<bool> frameCompleted{ true };
};


class Renderer
{
public:
	Renderer();

	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);
	void Finalize();

	std::shared_ptr<RootPipeline> GetRootPipeline() { return m_rootPipeline; }

	std::shared_ptr<RenderTargetView> GetBackBuffer();

	void Render();
	void WaitForPreviousFrame();

	// Factory methods
	std::shared_ptr<IndexBuffer> CreateIndexBuffer(std::unique_ptr<IIndexBufferData> data, Usage usage, const std::string& debugName);
	std::shared_ptr<VertexBuffer> CreateVertexBuffer(std::unique_ptr<IVertexBufferData> data, Usage usage, const std::string& debugName);

private:
	void StartRenderTask();
	void StopRenderTask();

	void EnqueueTask(std::shared_ptr<IAsyncRenderTask> task);

private:
	// Graphics API specific resources
	std::unique_ptr<DeviceResources>				m_deviceResources{ nullptr };

	// Resource managers
	std::unique_ptr<CommandListManager>				m_commandListManager{ nullptr };

	// Async render task and task queue
	RenderTaskEnvironment												m_renderTaskEnvironment;
	Concurrency::task<void>												m_renderTask;
	Concurrency::concurrent_queue<std::shared_ptr<IAsyncRenderTask>>	m_renderTaskQueue;
	bool																m_renderTaskStarted{ false };

	// Root render pipeline - the start of the 3D scene render
	std::shared_ptr<RootPipeline>					m_rootPipeline;
};

} // namespace Kodiak