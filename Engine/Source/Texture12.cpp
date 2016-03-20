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

#include "DDSTextureLoader12.h"
#include "DeviceManager12.h"
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


void Texture::LoadInternal(shared_ptr<Texture> texture, bool sRGB, const string& path)
{
	TextureFormat format = TextureFormat::None;

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

		if (format != TextureFormat::None)
		{
			string fullPath = Paths::GetInstance().TextureDir() + path;
			auto deviceManager = Renderer::GetInstance().GetDeviceManager();

			switch (format)
			{
			case TextureFormat::DDS:
				
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
}