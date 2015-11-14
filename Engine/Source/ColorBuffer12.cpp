// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ColorBuffer.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "ColorBuffer12.h"

#include "DeviceManager.h"
#include "DXGIUtility.h"

using namespace Kodiak;
using namespace std;


ColorBuffer::ColorBuffer(const DirectX::XMVECTORF32& clearColor)
	: m_clearColor(clearColor)
{
	m_srvHandle.ptr = ~0ull;
	m_rtvHandle.ptr = ~0ull;
	m_uavHandle.ptr = ~0ull;
}


void ColorBuffer::CreateFromSwapChain(DeviceManager* deviceManager, const string& name, ID3D12Resource* baseResource)
{
	AssociateWithResource(name, baseResource, D3D12_RESOURCE_STATE_PRESENT);

	// BUG:  Currently, we are prohibited from creating UAVs of the swap chain.  We can create an SRV, but we
	// don't have a need other than generality.
	//CreateDerivedViews(Graphics::g_Device, BaseResource->GetDesc().Format, 1);

	// WORKAROUND:  Just create a typical RTV
	m_rtvHandle = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	deviceManager->GetDevice()->CreateRenderTargetView(m_resource.Get(), nullptr, m_rtvHandle);
}


void ColorBuffer::Create(DeviceManager* deviceManager, const std::string& name, size_t width, size_t height, size_t depthOrArraySize, ColorFormat format)
{
	auto resourceDesc = DescribeTex2D(width, height, depthOrArraySize, format,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = resourceDesc.Format;
	clearValue.Color[0] = m_clearColor.f[0];
	clearValue.Color[1] = m_clearColor.f[1];
	clearValue.Color[2] = m_clearColor.f[2];
	clearValue.Color[3] = m_clearColor.f[3];

	CreateTextureResource(name, resourceDesc, clearValue);
	CreateDerivedViews(deviceManager, depthOrArraySize);
}


void ColorBuffer::CreateDerivedViews(DeviceManager* deviceManager, size_t arraySize)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	rtvDesc.Format = m_format;
	uavDesc.Format = DXGIUtility::GetUAVFormat(m_format);
	srvDesc.Format = m_format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if(arraySize > 1)
	{
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;
		rtvDesc.Texture2DArray.ArraySize = (UINT)arraySize;

		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = (UINT)arraySize;

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = (UINT)arraySize;
	}
	else
	{
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}

	if (~0ull == m_srvHandle.ptr)
	{
		m_rtvHandle = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_uavHandle = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_srvHandle = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	auto device = deviceManager->GetDevice();
	ID3D12Resource* resource = m_resource.Get();

	// Create the render target view
	device->CreateRenderTargetView(resource, &rtvDesc, m_rtvHandle);

	// Create the UAV (RWTexture2D)
	device->CreateUnorderedAccessView(resource, nullptr, &uavDesc, m_uavHandle);

	// Create the shader resource view
	device->CreateShaderResourceView(resource, &srvDesc, m_srvHandle);
}