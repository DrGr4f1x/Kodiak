// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Texture.h"

#include "Format.h"
#include "ResourceLoader.h"
#include "TextureResource.h"


using namespace Kodiak;
using namespace std;


uint32_t Texture::GetWidth() const
{
	return m_resource ? m_resource->GetWidth() : 0;
}


uint32_t Texture::GetHeight() const
{
	return m_resource ? m_resource->GetHeight() : 0;
}


uint32_t Texture::GetDepth() const
{
	return m_resource ? m_resource->GetDepth() : 0;
}


uint32_t Texture::GetArraySize() const
{
	return m_resource ? m_resource->GetArraySize() : 0;
}


uint32_t Texture::GetMipLevels() const
{
	return m_resource ? m_resource->GetMipLevels() : 0;
}


ColorFormat Texture::GetFormat() const
{
	return m_resource ? m_resource->GetFormat() : ColorFormat::Unknown;
}


bool Texture::IsReady() const
{
	return m_resource ? m_resource->IsReady() : false;
}


ShaderResourceView Texture::GetSRV()
{
	assert(m_resource && m_resource->IsReady());

	return m_resource->GetSRV();
}


shared_ptr<Texture> Texture::Create(uint32_t width, uint32_t height, ColorFormat format, const void* initData)
{
	auto texture = make_shared<Texture>();

	texture->m_name = "<Unnamed Texture>";
	
	texture->m_resource = make_shared<TextureResource>();
	texture->m_resource->Create(width, height, format, initData);

	return texture;
}


shared_ptr<Texture> Texture::Create(const string& name, uint32_t width, uint32_t height, ColorFormat format, const void* initData)
{
	auto texture = make_shared<Texture>();

	texture->m_name = name;

	texture->m_resource = make_shared<TextureResource>();
	texture->m_resource->Create(width, height, format, initData);

	return texture;
}


shared_ptr<Texture> Texture::CreateArray(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData)
{
	auto texture = make_shared<Texture>();

	texture->m_name = "<Unnamed Texture Array>";

	texture->m_resource = make_shared<TextureResource>();
	texture->m_resource->CreateArray(width, height, arraySize, numMips, format, initData);

	return texture;
}


shared_ptr<Texture> Texture::CreateArray(const string& name, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData)
{
	auto texture = make_shared<Texture>();

	texture->m_name = name;

	texture->m_resource = make_shared<TextureResource>();
	texture->m_resource->CreateArray(width, height, arraySize, numMips, format, initData);

	return texture;
}


shared_ptr<Texture> Texture::Load(const string& path, bool isSRGB)
{
	auto texture = make_shared<Texture>();

	texture->m_name = "<Unnamed Texture>";

	texture->m_resource = ResourceLoader<TextureResource>::GetInstance().Load(path, isSRGB);

	return texture;
}


void Texture::AddPostLoadCallback(function<void()> callback)
{
	assert(m_resource);

	m_resource->AddPostLoadCallback(callback);
}