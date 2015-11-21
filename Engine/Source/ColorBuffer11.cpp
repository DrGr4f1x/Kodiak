// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ColorBuffer11.h"

#include "DeviceManager.h"
#include "DXGIUtility.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


ColorBuffer::ColorBuffer(const DirectX::XMVECTORF32& clearColor)
	: m_clearColor(clearColor)
{}


void ColorBuffer::CreateFromSwapChain(DeviceManager* deviceManager, const string& name, ID3D11Texture2D* baseTexture)
{
	AssociateWithTexture(name, baseTexture);

	auto device = deviceManager->GetDevice();
	ThrowIfFailed(device->CreateRenderTargetView(baseTexture, nullptr, &m_rtv));
}


void ColorBuffer::Create(DeviceManager* deviceManager, const std::string& name, size_t width, size_t height, size_t depthOrArraySize, ColorFormat format)
{
	auto textureDesc = DescribeTex2D(width, height, depthOrArraySize, format,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS);

	CreateTextureResource(name, textureDesc);
	CreateDerivedViews(depthOrArraySize);
}


void ColorBuffer::CreateDerivedViews(size_t arraySize)
{
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = m_format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	ThrowIfFailed(g_device->CreateRenderTargetView(m_resource.Get(), &rtvDesc, &m_rtv));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGIUtility::GetUAVFormat(m_format);
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	ThrowIfFailed(g_device->CreateUnorderedAccessView(m_resource.Get(), &uavDesc, &m_uav));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = m_format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, &m_srv));
}