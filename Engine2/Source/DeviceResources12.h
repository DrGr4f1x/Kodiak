#pragma once

namespace Kodiak
{

// Forward declarations
class CommandList;
class RenderTargetView;

class DeviceResources
{
public:
	DeviceResources();

	void HandleDeviceLost();
	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);
	void Finalize();

	std::shared_ptr<RenderTargetView> GetBackBuffer();
	void Present();
	void WaitForGPU();

	std::shared_ptr<CommandList> CreateCommandList();
	void ExecuteCommandList(const std::shared_ptr<CommandList>& commandList);
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator();

private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();
	void MoveToNextFrame();

private:
	static const uint32_t FrameCount = 2;

	// Direct3D objects.
	Microsoft::WRL::ComPtr<ID3D12Device>			m_device;
	Microsoft::WRL::ComPtr<ID3D11On12Device>		m_d3d11On12Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		m_d3d11DeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_swapChain;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	m_commandAllocators[FrameCount];
	std::shared_ptr<RenderTargetView>				m_backbuffers[FrameCount];
	uint32_t m_rtvDescriptorSize;

	// Direct2D drawing components.
	Microsoft::WRL::ComPtr<ID2D1Factory3>			m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device2>			m_d2dDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext2>		m_d2dDeviceContext;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1>			m_d2dBitmapTargets[FrameCount];

	// DirectWrite drawing components.
	Microsoft::WRL::ComPtr<IDWriteFactory2>			m_dwriteFactory;
	Microsoft::WRL::ComPtr<IWICImagingFactory2>		m_wicFactory;

	// Synchronization objects.
	HANDLE									m_fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence>		m_fence;
	uint64_t								m_fenceValues[FrameCount];

	// Cached device properties.
	uint32_t								m_currentFrame{ 0 };
	HWND									m_hwnd{ nullptr };
	uint32_t								m_width{ 1 };
	uint32_t								m_height{ 1 };
	bool									m_deviceRemoved{ false };
};

} // namespace Kodiak