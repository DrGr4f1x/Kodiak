// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "TextureResource.h"

#include "CommandList12.h"
#include "DDSTextureLoader12.h"
#include "DeviceManager12.h"
#include "DXGIUtility.h"
#include "Filesystem.h"
#include "Format.h"
#include "LoaderEnums.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace std;


namespace
{

enum class TextureFormat : uint32_t
{
	None,
	DDS,

	NumFormats
};

const string s_formatString[] =
{
	"none",
	"dds",
};

} // anonymous namespace


TextureResource::TextureResource()
	: m_format(ColorFormat::Unknown)
	, m_loadState(LoadState::LoadNotStarted)
{
	InitializeSRV(m_srv);
}


TextureResource::TextureResource(bool isSRGB)
	: m_isSRGB(isSRGB)
	, m_format(ColorFormat::Unknown)
	, m_loadState(LoadState::LoadNotStarted)
{
	InitializeSRV(m_srv);
}


TextureResource::TextureResource(ShaderResourceView srv)
	: m_srv(srv)
	, m_format(ColorFormat::Unknown)
	, m_loadState(LoadState::LoadNotStarted)
{}


ColorFormat TextureResource::GetFormat() const
{
	return m_format;
}


void TextureResource::Create(uint32_t width, uint32_t height, ColorFormat format, const void* initData)
{
	m_usageState = D3D12_RESOURCE_STATE_COMMON;

	m_width = width;
	m_height = height;
	m_depth = 1;
	m_arraySize = 1;
	m_mipLevels = 1;
	m_format = format;

	DXGI_FORMAT dxgiFormat = DXGIUtility::ConvertToDXGI(format);

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = width;
	texDesc.Height = (UINT)height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = dxgiFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	ThrowIfFailed(g_device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
		m_usageState, nullptr, IID_PPV_ARGS(m_resource.ReleaseAndGetAddressOf())));

	m_resource->SetName(L"Texture");

	D3D12_SUBRESOURCE_DATA texResource;
	texResource.pData = initData;
	texResource.RowPitch = width * DXGIUtility::BytesPerPixel(dxgiFormat);
	texResource.SlicePitch = texResource.RowPitch * height;

	CommandList::InitializeTexture(*this, 1, &texResource);

	if (m_srv.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_srv = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	g_device->CreateShaderResourceView(m_resource.Get(), nullptr, m_srv);

	m_loadState = LoadState::LoadSucceeded;
}


void TextureResource::CreateArray(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData)
{
	m_usageState = D3D12_RESOURCE_STATE_COMMON;

	m_width = width;
	m_height = height;
	m_depth = 1;
	m_arraySize = arraySize;
	m_mipLevels = numMips;
	m_format = format;

	DXGI_FORMAT dxgiFormat = DXGIUtility::ConvertToDXGI(format);

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = width;
	texDesc.Height = (UINT)height;
	texDesc.DepthOrArraySize = arraySize;
	texDesc.MipLevels = numMips;
	texDesc.Format = dxgiFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Alignment = 0x10000;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	ThrowIfFailed(g_device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
		m_usageState, nullptr, IID_PPV_ARGS(m_resource.ReleaseAndGetAddressOf())));

	m_resource->SetName(L"Texture");

	if (initData)
	{
		D3D12_SUBRESOURCE_DATA texResource;
		texResource.pData = initData;
		texResource.RowPitch = width * DXGIUtility::BytesPerPixel(dxgiFormat);
		texResource.SlicePitch = texResource.RowPitch * height;

		CommandList::InitializeTexture(*this, 1, &texResource);
	}

	if (m_srv.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_srv = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = dxgiFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = 4;
	srvDesc.Texture2DArray.ArraySize = 16;

	g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv);

	m_loadState = LoadState::LoadSucceeded;
}


bool TextureResource::DoLoad()
{
	m_loadState = LoadState::Loading;
	
	TextureFormat format = TextureFormat::None;

	auto sepIndex = m_resourcePath.rfind('.');
	if (sepIndex != string::npos)
	{
		string extension = m_resourcePath.substr(sepIndex + 1);
		transform(begin(extension), end(extension), begin(extension), ::tolower);

		for (uint32_t i = 0; i < static_cast<uint32_t>(TextureFormat::NumFormats); ++i)
		{
			if (extension == s_formatString[i])
			{
				format = static_cast<TextureFormat>(i);
				break;
			}
		}
	}

	if (format == TextureFormat::None)
	{
		m_loadState = LoadState::LoadFailed;
		return false;
	}

	auto& filesystem = Filesystem::GetInstance();
	string fullpath = filesystem.GetFullPath(m_resourcePath);
	assert(!fullpath.empty());

	switch (format)
	{
	case TextureFormat::DDS:

		m_srv = DeviceManager::GetInstance().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		ThrowIfFailed(CreateDDSTextureFromFile(g_device,
			fullpath,
			0, // maxsize
			m_isSRGB,
			m_resource.GetAddressOf(),
			m_srv));
		break;
	}

	m_loadState = LoadState::LoadSucceeded;
	return true;
}