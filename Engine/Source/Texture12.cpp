// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Texture12.h"

#include "CommandList12.h"
#include "DDSTextureLoader12.h"
#include "DXGIUtility.h"
#include "DeviceManager12.h"
#include "Format.h"
#include "Paths.h"
#include "Renderer.h"
#include "RenderUtils.h"


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
	m_usageState = D3D12_RESOURCE_STATE_COMMON;

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

	if (m_cpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_cpuDescriptorHandle = Renderer::GetInstance().GetDeviceManager()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	g_device->CreateShaderResourceView(m_resource.Get(), nullptr, m_cpuDescriptorHandle);
}


void Texture::LoadInternal(shared_ptr<Texture> texture, bool sRGB, const string& path)
{
	TextureFormat format = TextureFormat::None;

	string extension;
	bool appendExtension = false;

	auto sepIndex = path.rfind('.');
	if (sepIndex != string::npos)
	{
		string extension = path.substr(sepIndex + 1);
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
		string fullPath = Paths::GetInstance().TextureDir() + path;
		if (appendExtension)
		{
			fullPath += extension;
		}

		switch (format)
		{
		case TextureFormat::DDS:
				
			auto deviceManager = Renderer::GetInstance().GetDeviceManager();
			texture->m_cpuDescriptorHandle = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			ThrowIfFailed(CreateDDSTextureFromFile(g_device,
				fullPath,
				0, // maxsize
				sRGB,
				texture->m_resource.GetAddressOf(),
				texture->m_cpuDescriptorHandle));
			break;
		}
	}
}