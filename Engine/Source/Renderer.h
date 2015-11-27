// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once



namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandList;
class DepthBuffer;
class DeviceManager;
class IAsyncRenderTask;
class Model;
class Pipeline;
class Scene;

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

} // namespace Kodiak


namespace Renderer
{

void Initialize();
void Finalize();

void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
void SetWindowSize(uint32_t width, uint32_t height);

void Render();

std::shared_ptr<Kodiak::Pipeline> GetRootPipeline();

void AddModel(std::shared_ptr<Kodiak::Scene> scene, std::shared_ptr<Kodiak::Model> model);
void UpdateModelTransform(std::shared_ptr<Kodiak::Model> model, const DirectX::XMFLOAT4X4& matrix);

} // namespace Renderer