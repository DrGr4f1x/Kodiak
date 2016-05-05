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

	std::function<void()> Render;

	void Continue(const std::shared_ptr<RenderTask> nextPipeline);
	void Start(Concurrency::task<void>& currentTask);

	void SetEnabled(bool enabled) { m_enabled = enabled; }

protected:
	std::string m_name;
	
	std::vector<std::shared_ptr<RenderTask>> m_antecedents;
	std::vector<std::shared_ptr<RenderTask>> m_predecessors;
	std::vector<concurrency::task<void>> m_predecessorTasks;

	bool m_enabled{ true };
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