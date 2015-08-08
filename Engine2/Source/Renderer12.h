#pragma once

namespace Kodiak
{

class CommandList;
class RenderTargetView;

class Renderer
{
public:
	void Initialize(uint32_t width, uint32_t height, HWND hwnd);
	void Finalize();

	std::shared_ptr<RenderTargetView> GetBackBuffer();
	void Present(bool bWaitForPreviousFrame);

	std::shared_ptr<CommandList> CreateCommandList();
	void ExecuteCommandList(const std::shared_ptr<CommandList>& commandList);


private:
	void WaitForPreviousFrame();

private:
	static const uint32_t FrameCount = 2;

	// Pipeline objects
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	std::shared_ptr<RenderTargetView> m_backbuffer[FrameCount];
	uint32_t m_rtvDescriptorSize;

	uint32_t m_frameIndex;

	// Synchronization objects.
	HANDLE m_fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	uint64_t m_fenceValue;
};

} // namespace Kodiak