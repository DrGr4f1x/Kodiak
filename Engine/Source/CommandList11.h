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
	void Present(const std::shared_ptr<RenderTargetView>& rtv) {}

	void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color);

	void SetCurrentFrame(uint32_t currentFrame) {}

public:

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deferredContext;
	Microsoft::WRL::ComPtr<ID3D11CommandList>	m_commandList;

	bool m_bIsRecording{ false };
};

} // namespace Kodiak