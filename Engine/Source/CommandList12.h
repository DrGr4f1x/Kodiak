#pragma once

namespace Kodiak
{

// Forward declarations
class DeviceResources;
class RenderTargetView;

class CommandList
{
public:
	void Begin();
	void End();
	void Present(const std::shared_ptr<RenderTargetView>& rtv);

	void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color);

public:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	DeviceResources* m_deviceResources{ nullptr };
};

} // namespace Kodiak
