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
	~CommandList();

	void Begin();
	void End();
	void Present(const std::shared_ptr<RenderTargetView>& rtv);

	void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color);

	void SetCurrentFrame(uint32_t currentFrame);
	
	ID3D12GraphicsCommandList* GetCommandList();
	void WaitForGpu();

public:
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>>	m_commandLists;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>		m_commandAllocators;
	uint32_t	m_currentFrame{ 0 };
	HANDLE		m_fenceEvent;
};

} // namespace Kodiak
