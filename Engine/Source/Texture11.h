// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "GpuResource11.h"

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
enum class ColorFormat;


class Texture : public GpuResource
{
	friend class GraphicsCommandList;

public:
	Texture() : m_srv() {}
	Texture(ID3D11ShaderResourceView* srv) : m_srv(srv) {}

	ID3D11ShaderResourceView* GetSRV() { return m_srv.Get(); }
	const ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }

	bool operator!() { return nullptr == m_srv.Get(); }

	static std::shared_ptr<Texture> Load(const std::string& path, bool sRGB, bool asyncLoad = true);
	void Create(uint32_t width, uint32_t height, ColorFormat format, const void* initData);

	concurrency::task<void> loadTask;

private:
	static void LoadInternal(std::shared_ptr<Texture> texture, bool sRGB, const std::string& path);

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_srv;
};

} // namespace Kodiak