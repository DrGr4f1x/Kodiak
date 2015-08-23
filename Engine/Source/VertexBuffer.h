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

class IVertexBufferData
{
public:
	virtual ~IVertexBufferData() {}

	virtual const uint8_t* GetData() const = 0;
	virtual const size_t GetSize() const = 0;
	virtual const size_t GetStride() const = 0;
};


template <class VertexType>
class TVertexBufferData : public IVertexBufferData
{
public:
	TVertexBufferData(std::initializer_list<VertexType> initializer) : m_data(initializer) {}

	const uint8_t* GetData() const override
	{
		if (!m_data.empty())
		{
			return reinterpret_cast<const uint8_t*>(&m_data[0]);
		}
		return nullptr;
	}

	const size_t GetSize() const override { return sizeof(VertexType) * m_data.size(); }
	const size_t GetStride() const override { return sizeof(VertexType); }

private:
	std::vector<VertexType> m_data;
};

} // namespace Kodiak