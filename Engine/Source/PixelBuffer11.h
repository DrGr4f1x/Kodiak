// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "GpuResource11.h"

namespace Kodiak
{

// Forward declarations
class DeviceManager;
enum class ColorFormat;
enum class DepthFormat;

class PixelBuffer : public GpuResource
{
public:
	uint32_t GetWidth()			const { return m_width; }
	uint32_t GetHeight()			const { return m_height; }
	uint32_t GetArraySize()		const { return m_arraySize; }
	DXGI_FORMAT GetFormat()		const { return m_format; }

protected:
	D3D11_TEXTURE2D_DESC DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, 
		ColorFormat format, uint32_t flags);
	D3D11_TEXTURE2D_DESC DescribeDepthTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, DepthFormat format, uint32_t flags);

	void AssociateWithTexture(const std::string& name, ID3D11Texture2D* texture);

	void CreateTextureResource(const std::string& name, const D3D11_TEXTURE2D_DESC& textureDesc);

protected:
	uint32_t		m_width{ 1 };
	uint32_t		m_height{ 1 };
	uint32_t		m_arraySize{ 1 };
	DXGI_FORMAT		m_format{ DXGI_FORMAT_UNKNOWN };

private:
	D3D11_TEXTURE2D_DESC InternalDescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips,
		DXGI_FORMAT format, uint32_t flags);
};

} // namespace Kodiak
