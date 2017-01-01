// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
enum class ColorFormat;
class TextureResource;

class Texture
{
public:
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	uint32_t GetDepth() const;
	uint32_t GetArraySize() const;
	uint32_t GetMipLevels() const;
	ColorFormat GetFormat() const;

	const std::string& GetName() const { return m_name; }

	ShaderResourceView GetSRV();

	bool IsReady() const;

	static std::shared_ptr<Texture> Create(uint32_t width, uint32_t height, ColorFormat format, const void* initData);
	static std::shared_ptr<Texture> Create(const std::string& name, uint32_t width, uint32_t height, ColorFormat format, const void* initData);
	static std::shared_ptr<Texture> CreateArray(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData = nullptr);
	static std::shared_ptr<Texture> CreateArray(const std::string& name, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t numMips, ColorFormat format, const void* initData = nullptr);

	static std::shared_ptr<Texture> Load(const std::string& path, bool isSRGB);

	void AddPostLoadCallback(std::function<void()> callback);

private:
	std::shared_ptr<TextureResource> m_resource;
	std::string m_name;
};

} // namespace Kodiak