// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "ColorBuffer12.h"
#include "DescriptorHeap12.h"
#include "PipelineState12.h"
#include "RootSignature12.h"

#define SWAP_CHAIN_BUFFER_COUNT 3

namespace Kodiak
{

class GraphicsCommandList;

class DeviceManager
{
public:
	static DeviceManager& GetInstance();

	void SetWindow(uint32_t width, uint32_t height, HWND hwnd);
	void SetWindowSize(uint32_t width, uint32_t height);
	void Finalize();

	void BeginFrame() {}
	void Present(std::shared_ptr<ColorBuffer> presentSource);

	// Feature support queries
	bool SupportsTypedUAVLoad_R11G11B10_FLOAT() const {	return m_supportsTypedUAVLoad_R11G11B10_FLOAT; }

	D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count = 1);
	
	ID3D12Device* GetDevice() { return m_device.Get(); }

private:
	DeviceManager();

	void CreateDeviceIndependentResources();
	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();
	void CreatePresentState();

	void PreparePresent(GraphicsCommandList* commandList, std::shared_ptr<ColorBuffer> presentSource);

private:
	// Direct3D objects
	Microsoft::WRL::ComPtr<ID3D12Device>			m_device;
	Microsoft::WRL::ComPtr<ID3D11On12Device>		m_d3d11On12Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		m_d3d11DeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_swapChain;

	// Direct2D drawing components.
	Microsoft::WRL::ComPtr<ID2D1Factory3>			m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device2>			m_d2dDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext2>		m_d2dDeviceContext;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1>			m_d2dBitmapTargets[SWAP_CHAIN_BUFFER_COUNT];

	// DirectWrite drawing components.
	Microsoft::WRL::ComPtr<IDWriteFactory2>			m_dwriteFactory;
	Microsoft::WRL::ComPtr<IWICImagingFactory2>		m_wicFactory;

	// Backbuffers
	ColorBuffer	m_backbuffers[SWAP_CHAIN_BUFFER_COUNT];

	// Cached device properties.
	uint32_t								m_currentFrame{ 0 };
	HWND									m_hwnd{ nullptr };
	uint32_t								m_width{ 1 };
	uint32_t								m_height{ 1 };
	bool									m_deviceRemoved{ false };

	// Allocators/heaps
	DescriptorAllocator						m_descriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	// Graphics state for Present
	RootSignature m_presentRS;
	GraphicsPSO m_convertLDRToDisplayPSO;

	// Feature flags
	bool m_supportsTypedUAVLoad_R11G11B10_FLOAT{ false };
};

// Global device handle, for convenience
extern ID3D12Device* g_device;

} // namespace Kodiak