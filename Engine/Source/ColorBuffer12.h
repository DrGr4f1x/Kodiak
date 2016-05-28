// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ColorBuffer.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "PixelBuffer.h"

namespace Kodiak
{

// Forward declarations
class DeviceManager;
enum class ColorFormat;


class ColorBuffer : public PixelBuffer
{
public:
	ColorBuffer(const DirectX::XMVECTORF32& clearColor = DirectX::Colors::Black);

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain(DeviceManager* deviceManager, const std::string& name, ID3D12Resource* baseResource);

	// Create a color buffer
	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips, ColorFormat format);

	// Create a color buffer array
	void CreateArray(const std::string& name, uint32_t width, uint32_t height, uint32_t arraySize, ColorFormat format);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV() const { return m_rtvHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_srvHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV() const { return m_uavHandle[0]; }

	DirectX::XMVECTORF32 GetClearColor() const { return m_clearColor; }

private:
	void CreateDerivedViews(uint32_t arraySize, uint32_t numMips);

private:
	DirectX::XMVECTORF32			m_clearColor;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_rtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_srvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_uavHandle[12];
	uint32_t						m_numMipMaps;
};

} // namespace Kodiak