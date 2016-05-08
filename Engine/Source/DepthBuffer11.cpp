// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "DepthBuffer11.h"

#include "DeviceManager11.h"
#include "DXGIUtility.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


DepthBuffer::DepthBuffer(float clearDepth, uint32_t clearStencil)
	: m_clearDepth(clearDepth)
	, m_clearStencil(clearStencil)
{}


void DepthBuffer::Create(DeviceManager* deviceManager,const std::string& name, uint32_t width, uint32_t height, DepthFormat format)
{
	auto textureDesc = DescribeDepthTex2D(width, height, 1, format,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL);

	CreateTextureResource(name, textureDesc);
	CreateDerivedViews();
}


void DepthBuffer::CreateDerivedViews()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGIUtility::GetDSVFormat(m_format);
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	
	ThrowIfFailed(g_device->CreateDepthStencilView(m_resource.Get(), &dsvDesc, &m_dsv));

	dsvDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
	ThrowIfFailed(g_device->CreateDepthStencilView(m_resource.Get(), &dsvDesc, &m_dsvReadOnlyDepth));

	DXGI_FORMAT stencilReadFormat = DXGIUtility::GetStencilFormat(m_format);
	if (DXGI_FORMAT_UNKNOWN != stencilReadFormat)
	{
		dsvDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
		ThrowIfFailed(g_device->CreateDepthStencilView(m_resource.Get(), &dsvDesc, &m_dsvReadOnly));

		dsvDesc.Flags = D3D11_DSV_READ_ONLY_STENCIL;
		ThrowIfFailed(g_device->CreateDepthStencilView(m_resource.Get(), &dsvDesc, &m_dsvReadOnlyStencil));
	}
	else
	{
		m_dsvReadOnlyStencil = m_dsv;
		m_dsvReadOnly = m_dsvReadOnlyDepth;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGIUtility::GetDepthFormat(m_format);
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, &m_depthSRV));

	if(DXGI_FORMAT_UNKNOWN != stencilReadFormat)
	{
		srvDesc.Format = DXGIUtility::GetStencilFormat(m_format);
		ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, &m_stencilSRV));
	}
	else
	{
		m_stencilSRV = nullptr;
	}
}