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
	
	size_t GetNumElements() const { return m_numElements; }
	void SetDebugName(const std::string& name) { m_debugName = name; }
	const std::string& GetDebugName() const { return m_debugName; }

	size_t GetId() const { return m_id; }

protected:
	std::string		m_debugName;
	size_t			m_numElements{ 0 };
	size_t			m_id{ 1 };
};


class IndexBufferData16 : public BaseIndexBufferData
{
public:
	IndexBufferData16(std::initializer_list<uint16_t> initializer)
	{
		m_id = s_baseId++;

		m_numElements = initializer.size();
		m_data = (uint16_t*)_aligned_malloc(sizeof(uint16_t) * m_numElements, 16);

		assert(m_data);
		if (m_data)
		{
			memcpy(m_data, initializer.begin(), sizeof(uint16_t) * m_numElements);
		}
	}


	IndexBufferData16(uint8_t* data, size_t sizeInBytes)
	{
		m_id = s_baseId++;

		m_numElements = sizeInBytes / sizeof(uint16_t);
		m_data = (uint16_t*)_aligned_malloc(sizeof(uint16_t) * m_numElements, 16);

		assert(m_data);
		if (m_data)
		{
			memcpy(m_data, data, sizeInBytes);
		}
	}


	~IndexBufferData16()
	{
		_aligned_free(m_data);
	}


	size_t GetDataSize() const override
	{
		return sizeof(uint16_t) * m_numElements;
	}


	const void* GetData() const
	{
		return m_data;
	}


	DXGI_FORMAT GetFormat() const override { return DXGI_FORMAT_R16_UINT; }
	size_t GetElementSize() const override { return sizeof(uint16_t); }

private:
	uint16_t*	m_data{ nullptr };
	static std::atomic_size_t		s_baseId;
};


class IndexBufferData32 : public BaseIndexBufferData
{
public:
	IndexBufferData32(std::initializer_list<uint32_t> initializer)
	{
		m_id = s_baseId++;

		m_numElements = initializer.size();
		m_data = (uint32_t*)_aligned_malloc(sizeof(uint32_t) * m_numElements, 16);

		assert(m_data);
		if (m_data)
		{
			memcpy(m_data, initializer.begin(), sizeof(uint32_t) * m_numElements);
		}
	}


	IndexBufferData32(uint8_t* data, size_t sizeInBytes)
	{
		m_id = s_baseId++;

		m_numElements = sizeInBytes / sizeof(uint32_t);
		m_data = (uint32_t*)_aligned_malloc(sizeof(uint32_t) * m_numElements, 16);

		assert(m_data);
		if (m_data)
		{
			memcpy(m_data, data, sizeInBytes);
		}
	}


	~IndexBufferData32()
	{
		_aligned_free(m_data);
	}


	size_t GetDataSize() const override
	{
		return sizeof(uint32_t) * m_numElements;
	}


	const void* GetData() const
	{
		return m_data;
	}


	DXGI_FORMAT GetFormat() const override { return DXGI_FORMAT_R32_UINT; }
	size_t GetElementSize() const override { return sizeof(uint32_t); }

private:
	uint32_t*	m_data{ nullptr };
	static std::atomic_size_t		s_baseId;
};

} // namespace Kodiak