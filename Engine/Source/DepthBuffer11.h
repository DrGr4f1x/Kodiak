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
enum class DepthFormat;


class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float clearDepth = 0.0f, uint32_t clearStencil = 0);

	void Create(const std::string& name, uint32_t width, uint32_t height, DepthFormat format);

	DepthStencilView GetDSV() { return GetRawDSV(m_dsv); }
	DepthStencilView GetDSVReadOnly() { return GetRawDSV(m_dsvReadOnly); }
	DepthStencilView GetDSVReadOnlyDepth() { return GetRawDSV(m_dsvReadOnlyDepth); }
	DepthStencilView GetDSVReadOnlyStencil() { return GetRawDSV(m_dsvReadOnlyStencil); }
	ShaderResourceView GetDepthSRV() { return GetRawSRV(m_depthSRV); }
	ShaderResourceView GetStencilSRV() { return GetRawSRV(m_stencilSRV); }

	float GetClearDepth() const { return m_clearDepth; }
	uint32_t GetClearStencil() const { return m_clearStencil; }

private:
	void CreateDerivedViews();

private:
	DepthStencilViewPtr m_dsv;
	DepthStencilViewPtr m_dsvReadOnly;
	DepthStencilViewPtr m_dsvReadOnlyDepth;
	DepthStencilViewPtr m_dsvReadOnlyStencil;
	ShaderResourceViewPtr m_depthSRV;
	ShaderResourceViewPtr m_stencilSRV;

	float		m_clearDepth{ 0.0f };
	uint32_t	m_clearStencil{ 0 };
};

} // namespace Kodiak