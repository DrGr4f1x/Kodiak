// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
enum class ColorFormat;
enum class DepthFormat;


class RenderPass
{
public:
	RenderPass(const std::string& name);

	const std::string& GetName() const { return m_name; }
	size_t GetHash() const { return m_hash; }

	// TODO: Maybe move this stuff to a render target manager?
	void SetRenderTargetFormat(ColorFormat colorFormat, DepthFormat depthFormat, uint32_t msaaCount = 1, uint32_t msaaQuality = 1)
	{
		SetRenderTargetFormats(1, &colorFormat, depthFormat, msaaCount, msaaQuality);
	}
	void SetRenderTargetFormats(uint32_t numRTVs, const ColorFormat* colorFormats, DepthFormat depthFormat, uint32_t msaaCount = 1,
		uint32_t msaaQuality = 0);

	uint32_t GetNumRenderTargets() const { return m_numRenderTargets; }
	void GetRenderTargetFormat(ColorFormat& colorFormat, DepthFormat& depthFormat, uint32_t& msaaCount, uint32_t& msaaQuality);
	void GetRenderTargetFormats(uint32_t numRTVs, ColorFormat** colorFormats, DepthFormat& depthFormat, uint32_t& msaaCount, 
		uint32_t& msaaQuality);

private:
	std::string		m_name;
	size_t			m_hash;

	ColorFormat		m_colorFormats[8];
	DepthFormat		m_depthFormat;
	uint32_t		m_numRenderTargets;

	uint32_t		m_msaaCount;
	uint32_t		m_msaaQuality;
};

} // namespace Kodiak