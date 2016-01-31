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
class IAsyncRenderTask;
class Model;
class Pipeline;
class Scene;
class StaticModel;

class AddStaticModelAction;
class RemoveStaticModelAction;

enum class ColorFormat;
enum class DepthFormat;
enum class Usage;

struct RenderTaskEnvironment
{
	DeviceManager* deviceManager;
	std::shared_ptr<Pipeline> rootPipeline;
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

	void EnqueueTask(std::shared_ptr<Kodiak::IAsyncRenderTask> task);

	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);

	void Update();
	void Render();

	std::shared_ptr<Kodiak::Pipeline> GetRootPipeline() { return m_rootPipeline; }
	DeviceManager* GetDeviceManager() { return m_deviceManager.get(); }

	// Static model management
	void AddStaticModelToScene(std::shared_ptr<StaticModel> model, std::shared_ptr<Scene> scene);
	void RemoveStaticModelFromScene(std::shared_ptr<StaticModel> model, std::shared_ptr<Scene> scene);

private:
	void StartRenderTask();
	void StopRenderTask();

	void UpdateStaticModels();

private:
	std::shared_ptr<Pipeline>			m_rootPipeline{ nullptr };
	RenderTaskEnvironment				m_renderTaskEnvironment;
	bool								m_renderTaskStarted{ false };
	Concurrency::task<void>				m_renderTask;
	Concurrency::concurrent_queue<std::shared_ptr<IAsyncRenderTask>>	m_renderTaskQueue;
	std::unique_ptr<DeviceManager>		m_deviceManager;

	// Pending model actions
	concurrency::concurrent_queue<std::shared_ptr<AddStaticModelAction>> m_pendingStaticModelAdds;
	concurrency::concurrent_queue<std::shared_ptr<RemoveStaticModelAction>> m_pendingStaticModelRemovals;

	uint32_t m_numStaticModelAdds{ 256 };
	uint32_t m_numStaticModelRemovals{ 256 };
};

// TODO: Eliminate this
namespace RenderThread
{

// Model functions
void AddModel(std::shared_ptr<Kodiak::Scene> scene, std::shared_ptr<Kodiak::Model> model);
void UpdateModelTransform(std::shared_ptr<Kodiak::Model> model, const DirectX::XMFLOAT4X4& matrix);

} // namespace RenderThread

} // namespace Kodiak