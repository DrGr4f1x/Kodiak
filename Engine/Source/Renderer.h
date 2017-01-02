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


void EnqueueRenderCommand(std::function<void()>&& command);


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

	void EnableRenderThread(bool enable);
	bool IsRenderThreadRunning() const;
	void ToggleRenderThread();

	void EnqueueRenderCommand(std::function<void()>&& command);

	void Update();
	void Render();

private:
	void StartRenderThread();
	void StopRenderThread();

private:
	bool m_renderThreadRunning{ false };
	bool m_renderThreadStateChanged{ false };
	Concurrency::concurrent_queue<std::function<void()>> m_renderCommandQueue;
	std::future<void> m_renderThreadFuture;
};


} // namespace Kodiak