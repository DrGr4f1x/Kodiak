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
class ComputeKernel;
class DepthBuffer;
class GraphicsCommandList;
class RenderPass;
class Scene;
enum class ResourceState;


class RenderTask : public std::enable_shared_from_this<RenderTask>
{
public:
	void SetName(const std::string& name);
	
	void ClearColor(std::shared_ptr<ColorBuffer> colorBuffer);
	void ClearColor(std::shared_ptr<ColorBuffer> colorBuffer, const DirectX::XMVECTORF32& color);
	void ClearDepth(std::shared_ptr<DepthBuffer> depthBuffer);

	void SetRenderTarget(std::shared_ptr<ColorBuffer> colorBuffer, std::shared_ptr<DepthBuffer> depthBuffer);
	void SetDepthStencilTarget(std::shared_ptr<DepthBuffer> depthBuffer);
	void SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height);

	void UpdateScene(std::shared_ptr<Scene> scene);
	void RenderScenePass(std::shared_ptr<RenderPass> renderPass, std::shared_ptr<Scene> scene);

	void TransitionResource(std::shared_ptr<ColorBuffer> resource, ResourceState newState, bool flushImmediate = false);
	void TransitionResource(std::shared_ptr<DepthBuffer> resource, ResourceState newState, bool flushImmediate = false);

	void Dispatch(std::shared_ptr<ComputeKernel> kernel, size_t groupCountX = 1, size_t groupCountY = 1, size_t groupCountZ = 1);
	void Dispatch1D(std::shared_ptr<ComputeKernel> kernel, size_t threadCountX, size_t groupSizeX = 64);
	void Dispatch2D(std::shared_ptr<ComputeKernel> kernel, size_t threadCountX, size_t threadCountY, size_t groupSizeX = 8, size_t groupSizeY = 8);
	void Dispatch3D(std::shared_ptr<ComputeKernel> kernel, size_t threadCountX, size_t threadCountY, size_t threadCountZ, size_t groupSizeX, size_t groupSizeY, size_t groupSizeZ);

	//void BeginGraphics();
	//void EndGraphics();

	void Continue(const std::shared_ptr<RenderTask> nextPipeline);
	void Start(Concurrency::task<void>& currentTask);
	
protected:
	void Run();

protected:
	std::string m_name;
	std::vector<std::function<void(GraphicsCommandList*)>>	m_renderSteps;

	std::vector<std::shared_ptr<RenderTask>> m_antecedents;
	std::vector<std::shared_ptr<RenderTask>> m_predecessors;
	std::vector<concurrency::task<void>> m_predecessorTasks;
};


class RootRenderTask : public RenderTask
{
public:
	void Start();
	void Wait();

	void Present(std::shared_ptr<ColorBuffer> colorBuffer);
	std::shared_ptr<ColorBuffer> GetPresentSource();

private:
	Concurrency::task<void> m_rootTask;

	std::shared_ptr<ColorBuffer>	m_presentSource;
};

} // namespace Kodiak