// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "PixelBuffer11.h"

#include "DeviceManager11.h"
#include "DXGIUtility.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace std;


D3D11_TEXTURE2D_DESC PixelBuffer::DescribeTex2D(size_t width, size_t height, size_t depthOrArraySize, ColorFormat format, uint32_t flags)
{
	return InternalDescribeTex2D(width, height, depthOrArraySize, DXGIUtility::ConvertToDXGI(format), flags);
}


D3D11_TEXTURE2D_DESC PixelBuffer::DescribeDepthTex2D(size_t width, size_t height, size_t depthOrArraySize, DepthFormat format, uint32_t flags)
{
	DXGI_FORMAT dxgiFormat = DXGIUtility::ConvertToDXGI(format);
	dxgiFormat = DXGIUtility::GetBaseFormat(dxgiFormat);
	return InternalDescribeTex2D(width, height, depthOrArraySize, dxgiFormat, flags);
}


void PixelBuffer::AssociateWithTexture(const std::string& name, ID3D11Texture2D* texture)
{
	assert(nullptr != texture);

	m_resource = texture;

	D3D11_TEXTURE2D_DESC resourceDesc;
	texture->GetDesc(&resourceDesc);

	m_width = resourceDesc.Width;
	m_height = resourceDesc.Height;
	m_arraySize = resourceDesc.ArraySize;
	m_format = resourceDesc.Format;

	m_resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str());
}


void PixelBuffer::CreateTextureResource(const std::string& name, const D3D11_TEXTURE2D_DESC& textureDesc)
{
	ID3D11Texture2D* texture = nullptr;

	ThrowIfFailed(g_device->CreateTexture2D(&textureDesc, nullptr, &texture));

	m_resource = texture;
	m_resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str());
}


D3D11_TEXTURE2D_DESC PixelBuffer::InternalDescribeTex2D(size_t width, size_t height, size_t depthOrArraySize, DXGI_FORMAT format, uint32_t flags)
{
	m_width = width;
	m_height = height;
	m_arraySize = depthOrArraySize;
	m_format = format;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = static_cast<UINT>(width);
	desc.Height = static_cast<UINT>(height);
	desc.MipLevels = 1;
	desc.ArraySize = static_cast<UINT>(depthOrArraySize);
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = flags;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	return desc;
}