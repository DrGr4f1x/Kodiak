// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
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
	void CreateFromSwapChain(DeviceManager* deviceManager, const std::string& name, BackBufferResource baseResource);

	// Create a color buffer
	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips, ColorFormat format);

	// Create a color buffer array
	void CreateArray(const std::string& name, uint32_t width, uint32_t height, uint32_t arraySize, ColorFormat format);

	RenderTargetView		GetRTV() { return GetRawRTV(m_rtv); }
	ShaderResourceView		GetSRV() { return GetRawSRV(m_srv); }
	UnorderedAccessView		GetUAV() { return GetRawUAV(m_uav[0]); }

	DirectX::XMVECTORF32 GetClearColor() const { return m_clearColor; }

private:
	void CreateDerivedViews(uint32_t arraySize, uint32_t numMips);

private:
	DirectX::XMVECTORF32	m_clearColor;
	RenderTargetViewPtr		m_rtv;
	ShaderResourceViewPtr	m_srv;
	UnorderedAccessViewPtr	m_uav[12];
	uint32_t				m_numMipMaps;
};

} // namespace Kodiak