#pragma once

#include <concurrent_queue.h>

namespace Kodiak
{

// Forward declarations
class CommandList;
class DeviceResources;
class RenderTargetView;

struct RenderTaskEnvironment 
{
	std::atomic<bool> stopRenderTask{ false };
};

typedef std::function<void(RenderTaskEnvironment&)> AsyncRenderTask;

class Renderer
{
public:
	Renderer();

	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);
	void Finalize();

	std::shared_ptr<RenderTargetView> GetBackBuffer();
	void Present();

	std::shared_ptr<CommandList> CreateCommandList();
	void ExecuteCommandList(const std::shared_ptr<CommandList>& commandList);

private:
	void StartRenderTask();
	void StopRenderTask();

private:
	// Graphics API specific resources
	std::unique_ptr<DeviceResources> m_deviceResources{ nullptr };

	// Async render task and task queue
	RenderTaskEnvironment							m_renderTaskEnvironment;
	Concurrency::task<void>							m_renderTask;
	Concurrency::concurrent_queue<AsyncRenderTask>	m_renderTaskQueue;
	bool											m_renderTaskStarted{ false };
};

} // namespace Kodiak