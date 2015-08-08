#pragma once

namespace Kodiak
{

// Forward declarations
class RenderTargetView;


class CommandList
{
public:
	void Begin();
	void End();

	void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color);

	// Unused in DX11
	void SynchronizeRenderTargetViewForRendering(const std::shared_ptr<RenderTargetView>& rtv) {}
	void SynchronizeRenderTargetViewForPresent(const std::shared_ptr<RenderTargetView>& rtv) {}

private:
	friend class Renderer;

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deferredContext;
	Microsoft::WRL::ComPtr<ID3D11CommandList>	m_commandList;

	bool m_bIsRecording{ false };
};

} // namespace Kodiak