// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "GpuBuffer11.h"

#include "DeviceManager.h"
#include "RenderUtils.h"

using namespace Kodiak;
using namespace std;


void GpuBuffer::Destroy()
{
	GpuResource::Destroy();
}


void GpuBuffer::Create(const std::string& name, uint32_t numElements, uint32_t elementSize, const void* initialData)
{
	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	auto bufferDesc = DescribeBuffer();
	ID3D11Buffer* buffer = nullptr;

	if (initialData)
	{
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = initialData;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		ThrowIfFailed(g_device->CreateBuffer(&bufferDesc, &initData, &buffer));
	}
	else
	{
		ThrowIfFailed(g_device->CreateBuffer(&bufferDesc, nullptr, &buffer));
	}

	m_resource = buffer;

	CreateDerivedViews();
}


D3D11_BUFFER_DESC GpuBuffer::DescribeBuffer()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = m_bufferSize;
	desc.BindFlags = m_bindFlags;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = m_miscFlags;

	return desc;
}


void ByteAddressBuffer::CreateDerivedViews()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = m_bufferSize;
	srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;

	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv.GetAddressOf()));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

	ThrowIfFailed(g_device->CreateUnorderedAccessView(m_resource.Get(), &uavDesc, m_uav.GetAddressOf()));
}


void StructuredBuffer::CreateDerivedViews()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = m_bufferSize;

	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv.GetAddressOf()));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = m_elementCount;

	ThrowIfFailed(g_device->CreateUnorderedAccessView(m_resource.Get(), &uavDesc, m_uav.GetAddressOf()));
}


void TypedBuffer::CreateDerivedViews()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	srvDesc.Format = m_dataFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = m_bufferSize;

	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv.GetAddressOf()));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

	uavDesc.Format = m_dataFormat;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = m_elementCount;

	ThrowIfFailed(g_device->CreateUnorderedAccessView(m_resource.Get(), &uavDesc, m_uav.GetAddressOf()));
}