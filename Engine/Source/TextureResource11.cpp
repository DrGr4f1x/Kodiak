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

#include "DDSTextureLoader11.h"
#include "DeviceManager11.h"
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
	D3D11_TEXTURE2D_DESC desc{};
	
	DXGI_FORMAT dxgiFormat = DXGIUtility::ConvertToDXGI(format);

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = dxgiFormat;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initData;
	data.SysMemPitch = static_cast<UINT>(width * DXGIUtility::BytesPerPixel(dxgiFormat));
	data.SysMemSlicePitch = static_cast<UINT>(data.SysMemPitch * height);

	ID3D11Texture2D* texture = nullptr;
	ThrowIfFailed(g_device->CreateTexture2D(&desc, &data, &texture));

	m_resource = texture;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = dxgiFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv.GetAddressOf()));

	m_loadState = LoadState::LoadSucceeded;
}


void TextureResource::CreateArray(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData)
{
	D3D11_TEXTURE2D_DESC desc{};
	
	DXGI_FORMAT dxgiFormat = DXGIUtility::ConvertToDXGI(format);

	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = arraySize;
	desc.MipLevels = numMips;
	desc.Format = dxgiFormat;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	ID3D11Texture2D* texture = nullptr;

	if (initData)
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = initData;
		data.SysMemPitch = static_cast<UINT>(width * DXGIUtility::BytesPerPixel(dxgiFormat));
		data.SysMemSlicePitch = static_cast<UINT>(data.SysMemPitch * height);

		ThrowIfFailed(g_device->CreateTexture2D(&desc, &data, &texture));
	}
	else
	{
		ThrowIfFailed(g_device->CreateTexture2D(&desc, nullptr, &texture));
	}

	m_resource = texture;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = dxgiFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = numMips;
	srvDesc.Texture2DArray.ArraySize = arraySize;

	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv.GetAddressOf()));

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

		ThrowIfFailed(CreateDDSTextureFromFile(g_device,
			fullpath,
			0, // maxsize
			m_isSRGB,
			m_resource.GetAddressOf(),
			m_srv.GetAddressOf()));
		break;
	}

	m_loadState = LoadState::LoadSucceeded;
	return true;
}