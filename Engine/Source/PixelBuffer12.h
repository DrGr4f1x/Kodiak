// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from PixelBuffer.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "GpuResource12.h"

namespace Kodiak
{

// Forward declarations
class DeviceManager;
enum class ColorFormat;
enum class DepthFormat;

class PixelBuffer : public GpuResource
{
public:

	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
	size_t GetDepth() const { return m_arraySize; }
	const DXGI_FORMAT& GetFormat() const { return m_format; }

protected:
	D3D12_RESOURCE_DESC DescribeTex2D(size_t width, size_t height, size_t depthOrArraySize, ColorFormat format, uint32_t flags);
	D3D12_RESOURCE_DESC DescribeDepthTex2D(size_t width, size_t height, size_t depthOrArraySize, DepthFormat format, uint32_t flags);

	void AssociateWithResource(const std::string& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState);

	void CreateTextureResource(const std::string& name, const D3D12_RESOURCE_DESC& resourceDesc,
		D3D12_CLEAR_VALUE clearValue);

protected:
	size_t			m_width{ 1 };
	size_t			m_height{ 1 };
	size_t			m_arraySize{ 1 };
	DXGI_FORMAT		m_format{ DXGI_FORMAT_UNKNOWN };

private:
	D3D12_RESOURCE_DESC InternalDescribeTex2D(size_t width, size_t height, size_t depthOrArraySize, DXGI_FORMAT format, uint32_t flags);
};

} // namespace Kodiak