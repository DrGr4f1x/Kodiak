// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from PixelBuffer.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "PixelBuffer12.h"

#include "DeviceManager12.h"
#include "DXGIUtility.h"
#include "RenderUtils.h"

#include <locale>
#include <codecvt>


using namespace Kodiak;
using namespace std;


D3D12_RESOURCE_DESC PixelBuffer::DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips,
	ColorFormat format, uint32_t flags)
{
	return InternalDescribeTex2D(width, height, depthOrArraySize, numMips, DXGIUtility::ConvertToDXGI(format), flags);
}


D3D12_RESOURCE_DESC PixelBuffer::DescribeDepthTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, DepthFormat format, uint32_t flags)
{
	const uint32_t numMips = 1;
	return InternalDescribeTex2D(width, height, depthOrArraySize, numMips, DXGIUtility::ConvertToDXGI(format), flags);
}


void PixelBuffer::AssociateWithResource(const string& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState)
{
	assert(nullptr != resource);
	auto resourceDesc = resource->GetDesc();

	m_resource = resource;
	m_usageState = currentState;

	m_width = static_cast<uint32_t>(resourceDesc.Width);
	m_height = resourceDesc.Height;
	m_arraySize = resourceDesc.DepthOrArraySize;
	m_format = resourceDesc.Format;

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wide = converter.from_bytes(name);

	m_resource->SetName(wide.c_str());
}


void PixelBuffer::CreateTextureResource(const string& name, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_CLEAR_VALUE clearValue)
{
	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	ThrowIfFailed(
		g_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&clearValue,
			IID_PPV_ARGS(&m_resource))
		);
}


D3D12_RESOURCE_DESC PixelBuffer::InternalDescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, 
	DXGI_FORMAT format, uint32_t flags)
{
	m_width = width;
	m_height = height;
	m_arraySize = depthOrArraySize;
	m_format = format;

	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = (UINT16)depthOrArraySize;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Flags = (D3D12_RESOURCE_FLAGS)flags;
	desc.Format = DXGIUtility::GetBaseFormat(m_format);
	desc.Height = (UINT)height;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.MipLevels = numMips;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Width = (UINT64)width;
	return desc;
}