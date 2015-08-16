#pragma once

namespace Kodiak
{

// Forward declarations
class CommandList;
class DeviceResources;
class RenderTargetView;

class Renderer
{
public:
	Renderer();

	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);
	void Finalize();

	std::shared_ptr<RenderTargetView> GetBackBuffer();
	void Present();
	
	std::shared_ptr<CommandList> CreateCommandList();
	void ExecuteCommandList(const std::shared_ptr<CommandList>& commandList);
	
private:
	std::unique_ptr<DeviceResources> m_deviceResources{ nullptr };
};

} // namespace Kodiak