#pragma once

namespace Kodiak
{

// Forward declarations
class CommandList;
class DeviceResources;
class IRenderOperation;
class RenderTargetView;

class Pipeline
{
public:
	virtual ~Pipeline();

	void SetName(const std::string& name);
	void SetCommandList(const std::shared_ptr<CommandList>& commandList);

	void Begin();
	void End();

	void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const DirectX::XMVECTORF32& color);

	void Execute(DeviceResources* deviceResources);
	void Submit(DeviceResources* deviceResources);

protected:
	std::string m_name;
	std::shared_ptr<CommandList> m_commandList;
	std::vector<IRenderOperation*> m_renderOperations;
};


class RootPipeline : public Pipeline
{
public:
	void Present(const std::shared_ptr<RenderTargetView>& rtv);
};

} // namespace Kodiak