// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "PipelineState11.h"

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class DepthBuffer;
class GraphicsCommandList;


class DeviceManager
{
public:
	DeviceManager();

	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);
	void Finalize() {}

	void BeginFrame();
	void Present(std::shared_ptr<ColorBuffer> presentSource);

	// Accessors
	ID3D11Device* GetDevice() { return m_d3dDevice.Get(); }

private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();

	void CreatePresentState();

	void PreparePresent(GraphicsCommandList* commandList, std::shared_ptr<ColorBuffer> presentSource);

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

	// Direct2D drawing components.
	Microsoft::WRL::ComPtr<ID2D1Factory2>		m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device1>		m_d2dDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext1>	m_d2dContext;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1>		m_d2dTargetBitmap;

	// DirectWrite drawing components.
	Microsoft::WRL::ComPtr<IDWriteFactory2>		m_dwriteFactory;
	Microsoft::WRL::ComPtr<IWICImagingFactory2>	m_wicFactory;

	// Rendering objects
	std::shared_ptr<ColorBuffer>			m_backBuffer;
	
	// Cached device properties.
	D3D_FEATURE_LEVEL						m_d3dFeatureLevel{ D3D_FEATURE_LEVEL_9_1 };
	HWND									m_hwnd{ nullptr };
	uint32_t								m_width{ 1 };
	uint32_t								m_height{ 1 };
	bool									m_deviceRemoved{ false };

	// Graphics state for present
	GraphicsPSO m_convertLDRToDisplayPSO;
};

// Global device handle, for convenience
extern ID3D11Device* g_device;

} // namespace Kodiak