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

class ColorBuffer : public PixelBuffer
{
public:
	ColorBuffer(const DirectX::XMVECTORF32& clearColor = DirectX::Colors::Black);

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain(DeviceManager* deviceManager, const std::string& name, ID3D11Texture2D* baseTexture);

	// Create a color buffer
	void Create(DeviceManager* deviceManager, const std::string& name, size_t width, size_t height, size_t depthOrArraySize, ColorFormat format);

	ID3D11RenderTargetView*		GetRTV() { return m_rtv.Get(); }
	ID3D11ShaderResourceView*	GetSRV() { return m_srv.Get(); }
	ID3D11UnorderedAccessView*	GetUAV() { return m_uav.Get(); }

	DirectX::XMVECTORF32 GetClearColor() const { return m_clearColor; }

private:
	void CreateDerivedViews(size_t arraySize);

private:
	DirectX::XMVECTORF32 m_clearColor;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_rtv;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_srv;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	m_uav;
};

} // namespace Kodiak