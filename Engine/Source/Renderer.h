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
class Camera;
class ColorBuffer;
class CommandList;
class DepthBuffer;
class DeviceManager;
class RootRenderTask;
class Scene;
class StaticModel;

enum class ColorFormat;
enum class DepthFormat;
enum class Usage;


struct RenderTaskEnvironment
{
	DeviceManager* deviceManager;
	std::shared_ptr<RootRenderTask> rootTask;
	std::atomic<uint64_t> currentFrame{ 0 };
	std::atomic<bool> stopRenderTask{ false };
	std::atomic<bool> frameCompleted{ true };
};

// TODO: Move this shit elsewhere
std::shared_ptr<ColorBuffer> CreateColorBuffer(const std::string& name, uint32_t width, uint32_t height, uint32_t arraySize, ColorFormat format,
	const DirectX::XMVECTORF32& clearColor);

std::shared_ptr<DepthBuffer> CreateDepthBuffer(const std::string& name, uint32_t width, uint32_t height, DepthFormat format, float clearDepth = 1.0f,
	uint32_t clearStencil = 0);


class Renderer
{
public:
	static Renderer& GetInstance();

	void Initialize();
	void Finalize();

	void EnqueueTask(std::function<void(RenderTaskEnvironment&)> callback);

	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);

	void Render(std::shared_ptr<RootRenderTask> rootTask);

	DeviceManager* GetDeviceManager() { return m_deviceManager.get(); }

private:
	void StartRenderTask();
	void StopRenderTask();

	void UpdateStaticModels();

private:
	RenderTaskEnvironment				m_renderTaskEnvironment;
	bool								m_renderTaskStarted{ false };
	Concurrency::task<void>				m_renderTask;
	Concurrency::concurrent_queue<std::function<void(RenderTaskEnvironment&)>>	m_renderTaskQueue;
	std::unique_ptr<DeviceManager>		m_deviceManager;
};


} // namespace Kodiak