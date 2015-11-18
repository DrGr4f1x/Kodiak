// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "IAsyncRenderTask.h"

namespace Kodiak
{


// Forward declarations
class ColorBuffer;
class CommandList;
class DepthBuffer;
class DeviceResources;
class IRenderOperation;
class Scene;


class Pipeline
{
public:
	virtual ~Pipeline();

	void SetName(const std::string& name);
	
	void ClearColor(std::shared_ptr<ColorBuffer> colorBuffer);
	void ClearColor(std::shared_ptr<ColorBuffer> colorBuffer, const DirectX::XMVECTORF32& color);
	void ClearDepth(std::shared_ptr<DepthBuffer> depthBuffer);

	void SetRenderTarget(std::shared_ptr<ColorBuffer> colorBuffer, std::shared_ptr<DepthBuffer> depthBuffer);
	void SetViewport(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

	void RenderScene(std::shared_ptr<Scene> scene);

	void Present(std::shared_ptr<ColorBuffer> colorBuffer);
	std::shared_ptr<ColorBuffer> GetPresentSource();

	void Execute();

protected:
	std::string m_name;
	std::vector<IRenderOperation*>	m_renderOperations;

	std::shared_ptr<ColorBuffer>	m_presentSource;
};


class RenderPipelineTask : public IAsyncRenderTask
{
public:
	RenderPipelineTask(std::shared_ptr<Pipeline> pipeline);

	void Execute(RenderTaskEnvironment& environment) override;

private:
	std::shared_ptr<Pipeline> m_pipeline;
};

} // namespace Kodiak