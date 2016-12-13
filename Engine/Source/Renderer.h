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


struct PresentParameters
{
	float PaperWhite;
	float MaxBrightness;
	float ToeStrength;
	uint32_t DebugMode;
};


class Renderer
{
public:
	static Renderer& GetInstance();

	void Initialize();
	void Finalize();

	void EnableRenderThread(bool enable) { /* TODO */ }

	void EnqueueTask(std::function<void(RenderTaskEnvironment&)> callback);

	void Render(std::shared_ptr<RootRenderTask> rootTask, bool bHDRPresent, const PresentParameters& params);

private:
	void StartRenderTask();
	void StopRenderTask();

	void UpdateStaticModels();

private:
	RenderTaskEnvironment				m_renderTaskEnvironment;
	bool								m_renderTaskStarted{ false };
	Concurrency::task<void>				m_renderTask;
	Concurrency::concurrent_queue<std::function<void(RenderTaskEnvironment&)>>	m_renderTaskQueue;
};


} // namespace Kodiak