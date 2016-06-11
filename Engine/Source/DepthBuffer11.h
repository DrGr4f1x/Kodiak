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

	ID3D11DepthStencilView* GetDSV() { return m_dsv.Get(); }
	ID3D11DepthStencilView* GetDSVReadOnly() { return m_dsvReadOnly.Get(); }
	ID3D11DepthStencilView* GetDSVReadOnlyDepth() { return m_dsvReadOnlyDepth.Get(); }
	ID3D11DepthStencilView* GetDSVReadOnlyStencil() { return m_dsvReadOnlyStencil.Get(); }
	ID3D11ShaderResourceView* GetDepthSRV() { return m_depthSRV.Get(); }
	ID3D11ShaderResourceView* GetStencilSRV() { return m_stencilSRV.Get(); }

	float GetClearDepth() const { return m_clearDepth; }
	uint32_t GetClearStencil() const { return m_clearStencil; }

private:
	void CreateDerivedViews();

private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsvReadOnly{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsvReadOnlyDepth{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsvReadOnlyStencil{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthSRV{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_stencilSRV{ nullptr };

	float		m_clearDepth{ 0.0f };
	uint32_t	m_clearStencil{ 0 };
};

} // namespace Kodiak