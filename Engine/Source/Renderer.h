#pragma once

#include <concurrent_queue.h>

namespace Kodiak
{

// Forward declarations
class CommandList;
class CommandListManager;
class DeviceResources;
class RenderTargetView;
class RootPipeline;


struct RenderTaskEnvironment 
{
	DeviceResources* deviceResources;
	std::shared_ptr<RootPipeline> rootPipeline;
	std::atomic<bool> stopRenderTask{ false };
	std::atomic<bool> frameCompleted{ true };
};


typedef std::function<void(RenderTaskEnvironment&)> AsyncRenderTask;


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

private:
	void StartRenderTask();
	void StopRenderTask();

	void EnqueueTask(AsyncRenderTask& task);

private:
	// Graphics API specific resources
	std::unique_ptr<DeviceResources>				m_deviceResources{ nullptr };

	// Resource managers
	std::unique_ptr<CommandListManager>				m_commandListManager{ nullptr };

	// Async render task and task queue
	RenderTaskEnvironment							m_renderTaskEnvironment;
	Concurrency::task<void>							m_renderTask;
	Concurrency::concurrent_queue<AsyncRenderTask>	m_renderTaskQueue;
	bool											m_renderTaskStarted{ false };

	// Root render pipeline - the start of the 3D scene render
	std::shared_ptr<RootPipeline>					m_rootPipeline;
};

} // namespace Kodiak