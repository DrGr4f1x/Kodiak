// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ColorBuffer.h"

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


void ColorBuffer::Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips, ColorFormat format)
{
	const uint32_t arraySize = 1;
	auto textureDesc = DescribeTex2D(width, height, arraySize, numMips, format,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS);

	CreateTextureResource(name, textureDesc);
	CreateDerivedViews(arraySize, numMips);
}


void ColorBuffer::CreateArray(const std::string& name, uint32_t width, uint32_t height, uint32_t arraySize, ColorFormat format)
{
	const uint32_t numMips = 1;
	auto textureDesc = DescribeTex2D(width, height, arraySize, numMips, format,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS);

	CreateTextureResource(name, textureDesc);
	CreateDerivedViews(arraySize, numMips);
}


void ColorBuffer::CreateDerivedViews(uint32_t arraySize, uint32_t numMips)
{
	assert_msg(arraySize == 1 || numMips == 1, "We don't support auto-mips on texture arrays");

	m_numMipMaps = numMips - 1;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	rtvDesc.Format = m_format;
	uavDesc.Format = DXGIUtility::GetUAVFormat(m_format);
	srvDesc.Format = m_format;

	if (arraySize > 1)
	{
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;
		rtvDesc.Texture2DArray.ArraySize = arraySize;

		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = arraySize;

		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = numMips;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = arraySize;
	}
	else
	{
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = numMips;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}

	ThrowIfFailed(g_device->CreateRenderTargetView(m_resource.Get(), &rtvDesc, &m_rtv));
	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, &m_srv));

	for (uint32_t i = 0; i < numMips; ++i)
	{
		ThrowIfFailed(g_device->CreateUnorderedAccessView(m_resource.Get(), &uavDesc, &m_uav[i]));
		uavDesc.Texture2D.MipSlice++;
	}
}