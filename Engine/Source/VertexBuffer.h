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
	virtual const void* GetData() const = 0;

	size_t GetStride() const { return m_elementSize; }
	size_t GetDataSize() const { return m_elementSize * m_numElements; }
	size_t GetElementSize() const { return m_elementSize; }
	size_t GetNumElements() const { return m_numElements; }

	void SetDebugName(const std::string& name) { m_debugName = name; }
	const std::string& GetDebugName() const { return m_debugName; }
	
	size_t GetId() const { return m_id; }

protected:
	std::string		m_debugName;
	size_t			m_elementSize{ 0 };
	size_t			m_numElements{ 0 };
	size_t			m_id{ 1 };
};


template <class VertexType>
class VertexBufferData : public BaseVertexBufferData
{
public:
	VertexBufferData(std::initializer_list<VertexType> initializer)
	{
		m_id = s_baseId++;

		m_elementSize = sizeof(VertexType);
		m_numElements = initializer.size();
		m_data = (VertexType*)_aligned_malloc(m_elementSize * m_numElements, 16);

		assert(m_data);
		if (m_data)
		{
			memcpy(m_data, initializer.begin(), m_elementSize * m_numElements);
		}
	}

	~VertexBufferData()
	{
		_aligned_free(m_data);
	}

	const void* GetData() const override
	{
		return m_data;
	}

private:
	VertexType*						m_data{ nullptr };
	static std::atomic_size_t		s_baseId;
};

template <class VertexType>
std::atomic_size_t VertexBufferData<VertexType>::s_baseId = 0;

} // namespace Kodiak