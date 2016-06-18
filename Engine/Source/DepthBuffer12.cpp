// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from DepthBuffer.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "DepthBuffer.h"

#include "DeviceManager12.h"
#include "DXGIUtility.h"
#include "Renderer.h"

using namespace Kodiak;
using namespace std;


DepthBuffer::DepthBuffer(float clearDepth, uint32_t clearStencil)
	: m_clearDepth(clearDepth)
	, m_clearStencil(clearStencil)
{
	InitializeDSV(m_dsv);
	InitializeDSV(m_dsvReadOnly);
	InitializeDSV(m_dsvReadOnlyDepth);
	InitializeDSV(m_dsvReadOnlyStencil);
	InitializeSRV(m_depthSRV);
	InitializeSRV(m_stencilSRV);
}


void DepthBuffer::Create(const std::string& name, uint32_t width, uint32_t height, DepthFormat format)
{
	D3D12_RESOURCE_DESC ResourceDesc = DescribeDepthTex2D(width, height, 1, format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = DXGIUtility::GetDSVFormat(ResourceDesc.Format);
	ClearValue.DepthStencil.Depth = m_clearDepth;
	ClearValue.DepthStencil.Stencil = m_clearStencil;
	CreateTextureResource(name, ResourceDesc, ClearValue);
	CreateDerivedViews();
}


void DepthBuffer::CreateDerivedViews()
{
	ID3D12Resource* resource = m_resource.Get();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGIUtility::GetDSVFormat(m_format);
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (m_dsv.ptr == ~0ull)
	{
		m_dsv = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_dsvReadOnly = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	g_device->CreateDepthStencilView(resource, &dsvDesc, m_dsv);

	dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	g_device->CreateDepthStencilView(resource, &dsvDesc, m_dsvReadOnly);

	DXGI_FORMAT stencilReadFormat = DXGIUtility::GetStencilFormat(m_format);
	if(stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_dsvReadOnlyStencil.ptr == ~0ull)
		{
			m_dsvReadOnlyStencil = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			m_dsvReadOnly = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		g_device->CreateDepthStencilView(resource, &dsvDesc, m_dsvReadOnlyStencil);

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		g_device->CreateDepthStencilView(resource, &dsvDesc, m_dsvReadOnlyDepth);
	}
	else
	{
		m_dsvReadOnlyStencil = m_dsv;
		m_dsvReadOnly = m_dsvReadOnlyDepth;
	}

	if (m_depthSRV.ptr == ~0ull)
	{
		m_depthSRV = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Create the shader resource view
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGIUtility::GetDepthFormat(m_format);
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	g_device->CreateShaderResourceView(resource, &srvDesc, m_depthSRV);

	if(stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_stencilSRV.ptr == ~0ull)
		{
			m_stencilSRV = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		srvDesc.Format = stencilReadFormat;
		g_device->CreateShaderResourceView(resource, &srvDesc, m_stencilSRV);
	}
}