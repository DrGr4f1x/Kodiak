// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "GpuResource.h"
#include "IAsyncResource.h"

namespace Kodiak
{


// Forward declarations
enum class ColorFormat;
enum class LoadState;


class TextureResource : public GpuResource, public IAsyncResource
{
public:
	TextureResource();
	explicit TextureResource(bool isSRGB);
	explicit TextureResource(ShaderResourceView srv);

	ShaderResourceView GetSRV() { return GetRawSRV(m_srv); }

	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	uint32_t GetDepth() const { return m_depth; }
	uint32_t GetArraySize() const { return m_arraySize; }
	uint32_t GetMipLevels() const { return m_mipLevels; }
	ColorFormat GetFormat() const;

	void Create(uint32_t width, uint32_t height, ColorFormat format, const void* initData);
	void CreateArray(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData = nullptr);

	// IAsyncResource interface
	bool DoLoad() final override;

private:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depth{ 0 };
	uint32_t m_arraySize{ 0 };
	uint32_t m_mipLevels{ 0 };
	ColorFormat m_format;
	bool m_isSRGB{ false };

	ShaderResourceViewPtr m_srv;
};

} // namespace Kodiak