// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#if defined(DX12)
#include "IndexBuffer12.h"
#elif defined(DX11)
#include "IndexBuffer11.h"
#elif defined(VK)
#include "IndexBufferVk.h"
#else
#error No graphics API defined!
#endif

namespace Kodiak
{

class BaseIndexBufferData
{
public:
	virtual ~BaseIndexBufferData() {}

	virtual size_t GetDataSize() const = 0;
	virtual const void* GetData() const = 0;
	virtual DXGI_FORMAT GetFormat() const = 0;
	virtual size_t GetElementSize() const = 0;
	virtual size_t GetNumElements() const = 0;
};


class IndexBufferData16 : public BaseIndexBufferData
{
public:
	IndexBufferData16(std::initializer_list<uint16_t> initializer)
	{
		m_size = sizeof(uint16_t) * initializer.size();
		m_data = (uint16_t*)_aligned_malloc(m_size, 16);

		assert(m_data);
		if (m_data)
		{
			memcpy(m_data, initializer.begin(), m_size);
		}
	}

	~IndexBufferData16()
	{
		_aligned_free(m_data);
	}

	size_t GetDataSize() const override
	{
		return sizeof(uint16_t) * m_size;
	}

	const void* GetData() const
	{
		return m_data;
	}

	DXGI_FORMAT GetFormat() const override { return DXGI_FORMAT_R16_UINT; }
	size_t GetElementSize() const override { return sizeof(uint16_t); }
	size_t GetNumElements() const override { return m_size; }

private:
	uint16_t*	m_data{ nullptr };
	size_t		m_size{ 0 };
};


class IndexBufferData32 : public BaseIndexBufferData
{
public:
	IndexBufferData32(std::initializer_list<uint32_t> initializer)
	{
		m_size = sizeof(uint32_t) * initializer.size();
		m_data = (uint32_t*)_aligned_malloc(m_size, 16);

		assert(m_data);
		if (m_data)
		{
			memcpy(m_data, initializer.begin(), m_size);
		}
	}

	~IndexBufferData32()
	{
		_aligned_free(m_data);
	}

	size_t GetDataSize() const override
	{
		return sizeof(uint32_t) * m_size;
	}

	const void* GetData() const
	{
		return m_data;
	}

	DXGI_FORMAT GetFormat() const override { return DXGI_FORMAT_R32_UINT; }
	size_t GetElementSize() const override { return sizeof(uint32_t); }
	size_t GetNumElements() const override { return m_size; }

private:
	uint32_t*	m_data{ nullptr };
	size_t		m_size{ 0 };
};

} // namespace Kodiak