// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Texture11.h"

#include "DeviceManager11.h"
#include "DDSTextureLoader11.h"
#include "DXGIUtility.h"
#include "Format.h"
#include "Paths.h"
#include "RenderUtils.h"

#include <Shlwapi.h>

using namespace Kodiak;
using namespace std;


namespace
{
map<string, shared_ptr<Texture>>	s_textureMap;

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


shared_ptr<Texture> Texture::Load(const string& path, bool sRGB, bool asyncLoad)
{
	shared_ptr<Texture> texture;

	{
		static mutex textureMutex;
		lock_guard<mutex> CS(textureMutex);

		auto iter = s_textureMap.find(path);

		if (iter == s_textureMap.end())
		{
			if (!PathFileExistsA(path.c_str()))
			{
				return nullptr;
			}

			texture = make_shared<Texture>();
			s_textureMap[path] = texture;

			if (asyncLoad)
			{
				// Non-blocking asynchronous load
				texture->loadTask = concurrency::create_task([texture, sRGB, path]()
				{
					LoadInternal(texture, sRGB, path);
				});
			}
			else
			{
				// Blocking synchronous create
				texture->loadTask = concurrency::create_task([] {});
				LoadInternal(texture, sRGB, path);
			}
		}
		else
		{
			texture = iter->second;
		}
	}

	return texture;
}


void Texture::Create(uint32_t width, uint32_t height, ColorFormat format, const void* initData)
{
	loadTask = concurrency::create_task([] {});

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

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

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = dxgiFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	ThrowIfFailed(g_device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv.GetAddressOf()));
}


void Texture::CreateArray(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData)
{
	loadTask = concurrency::create_task([] {});

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

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
		ID3D11Texture2D* texture = nullptr;
		ThrowIfFailed(g_device->CreateTexture2D(&desc, nullptr, &texture));
	}

	m_resource = texture;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = dxgiFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = numMips;
	srvDesc.Texture2DArray.ArraySize = arraySize;
}


void Texture::LoadInternal(shared_ptr<Texture> texture, bool sRGB, const string& path)
{
	TextureFormat format = TextureFormat::None;

	string extension;
	bool appendExtension = false;

	extern ID3D11Device* g_device;

	auto sepIndex = path.rfind('.');
	if (sepIndex != string::npos)
	{
		extension = path.substr(sepIndex + 1);
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
	else
	{
		// Assume .dds if there is no extension
		format = TextureFormat::DDS;
		extension = ".dds";
		appendExtension = true;
	}

	if (format != TextureFormat::None)
	{
		string fullPath = path;
		if (appendExtension)
		{
			fullPath += extension;
		}

		switch (format)
		{
		case TextureFormat::DDS:
			ThrowIfFailed(CreateDDSTextureFromFile(g_device,
				fullPath,
				0, // maxsize
				sRGB,
				texture->m_resource.GetAddressOf(),
				texture->m_srv.GetAddressOf()));
			break;
		}
	}
}