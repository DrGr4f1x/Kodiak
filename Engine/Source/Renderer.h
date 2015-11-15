// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include <concurrent_queue.h>

namespace Kodiak
{

// Forward declarations
class BaseIndexBufferData;
class ColorBuffer;
class CommandList;
class DeviceManager;
class IAsyncRenderTask;
class IndexBuffer;
class Pipeline;
class RenderTargetView;

enum class ColorFormat;
enum class Usage;

struct RenderTaskEnvironment 
{
	DeviceManager* deviceManager;
	std::shared_ptr<Pipeline> rootPipeline;
	std::atomic<uint64_t> currentFrame{ 0 };
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

	std::shared_ptr<Pipeline> GetRootPipeline() { return m_rootPipeline; }

	bool Render();

	// Factory methods
	std::shared_ptr<ColorBuffer> CreateColorBuffer(const std::string& name, uint32_t width, uint32_t height, uint32_t arraySize, ColorFormat format,
		const DirectX::XMVECTORF32& clearColor);
	std::shared_ptr<IndexBuffer> CreateIndexBuffer(std::shared_ptr<BaseIndexBufferData> data, Usage usage, const std::string& debugName);

private:
	void StartRenderTask();
	void StopRenderTask();

	void EnqueueTask(std::shared_ptr<IAsyncRenderTask> task);

private:
	// Graphics API specific resources
	std::unique_ptr<DeviceManager>				m_deviceManager{ nullptr };

	// Async render task and task queue
	RenderTaskEnvironment												m_renderTaskEnvironment;
	Concurrency::task<void>												m_renderTask;
	Concurrency::concurrent_queue<std::shared_ptr<IAsyncRenderTask>>	m_renderTaskQueue;
	bool																m_renderTaskStarted{ false };

	// Root render pipeline - the start of the 3D scene render
	std::shared_ptr<Pipeline>					m_rootPipeline;
};

} // namespace Kodiak