#pragma once

namespace Kodiak
{

// Forward declarations
class CommandList;
class RenderTargetView;
class DepthStencilView;

class DeviceResources
{
public:
	DeviceResources();

	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);

	std::shared_ptr<CommandList> CreateCommandList();
	void ExecuteCommandList(const std::shared_ptr<CommandList>& commandList);

	std::shared_ptr<RenderTargetView> GetBackBuffer();
	void Present();

private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();

private:
	// Direct3D objects.
	Microsoft::WRL::ComPtr<ID3D11Device>			m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		m_d3dContext;
	Microsoft::WRL::ComPtr<ID3D11Device1>			m_d3dDevice1;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1>	m_d3dContext1;
	Microsoft::WRL::ComPtr<ID3D11Device2>			m_d3dDevice2;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext2>	m_d3dContext2;
	Microsoft::WRL::ComPtr<ID3D11Device3>			m_d3dDevice3;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext3>	m_d3dContext3;
	Microsoft::WRL::ComPtr<IDXGISwapChain1>			m_swapChain;

	// Rendering objects
	std::shared_ptr<RenderTargetView>			m_d3dRenderTargetView;
	std::shared_ptr<DepthStencilView>			m_d3dDepthStencilView;
	D3D11_VIEWPORT								m_screenViewport;

	// Direct2D drawing components.
	Microsoft::WRL::ComPtr<ID2D1Factory2>		m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device1>		m_d2dDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext1>	m_d2dContext;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1>		m_d2dTargetBitmap;

	// DirectWrite drawing components.
	Microsoft::WRL::ComPtr<IDWriteFactory2>		m_dwriteFactory;
	Microsoft::WRL::ComPtr<IWICImagingFactory2>	m_wicFactory;

	// Cached device properties.
	D3D_FEATURE_LEVEL						m_d3dFeatureLevel{ D3D_FEATURE_LEVEL_9_1 };
	HWND									m_hwnd{ nullptr };
	uint32_t								m_width{ 1 };
	uint32_t								m_height{ 1 };
	bool									m_deviceRemoved{ false };
};

} // namespace Kodiak