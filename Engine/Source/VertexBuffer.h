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
#include "VertexBuffer12.h"
#elif defined(DX11)
#include "VertexBuffer11.h"
#elif defined(VK)
#include "VertexBufferVk.h"
#else
#error No graphics API defined!
#endif


namespace Kodiak
{

class BaseVertexBufferData
{
public:
	virtual ~BaseVertexBufferData() {}
	virtual size_t GetDataSize() const = 0;
	virtual const void* GetData() const = 0;
	virtual size_t GetStride() const = 0;
	virtual size_t GetElementSize() const = 0;
	virtual size_t GetNumElements() const = 0;
};


template <class VertexType>
class VertexBufferData : public BaseVertexBufferData
{
public:
	VertexBufferData(std::initializer_list<VertexType> initializer)
	{
		m_size = sizeof(VertexType) * initializer.size();
		m_data = (VertexType*)_aligned_malloc(m_size, 16);
		memcpy(m_data, initializer.begin(), m_size);
	}

	~VertexBufferData()
	{
		_aligned_free(m_data);
	}

	size_t GetDataSize() const override
	{
		return sizeof(VertexType) * m_size;
	}

	const void* GetData() const override
	{
		return m_data;
	}

	size_t GetStride() const override
	{
		return sizeof(VertexType);
	}

	size_t GetElementSize() const override
	{
		return sizeof(VertexType);
	}

	size_t GetNumElements() const override
	{
		return m_size;
	}

private:
	VertexType* m_data{ nullptr };
	size_t		m_size{ 0 };
};

} // namespace Kodiak