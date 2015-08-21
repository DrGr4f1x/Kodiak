#pragma once

namespace Kodiak
{

// Forward declarations
class DeviceResources;
class RenderTargetView;

class CommandList
{
public:
	CommandList(uint32_t frameCount);

	void Begin();
	void End();
	void Present(const std::shared_ptr<RenderTargetView>& rtv);

	void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color);

	void SetCurrentFrame(uint32_t currentFrame);

public:
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>>	m_commandLists;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>		m_commandAllocators;
	
	uint32_t m_currentFrame{ 0 };
};

} // namespace Kodiak
