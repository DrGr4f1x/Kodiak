#pragma once

#include "RenderEnums.h"

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

class IIndexBufferData
{
public:
	virtual ~IIndexBufferData() {}

	virtual const uint8_t* GetData() const = 0;
	virtual const size_t GetSize() const = 0;
	virtual const IndexBufferFormat GetFormat() const = 0;
};


class IndexBufferData16 : public IIndexBufferData
{
public:
	IndexBufferData16(std::initializer_list<uint16_t> initializer) : m_data(initializer) {}

	const uint8_t* GetData() const override
	{
		if (!m_data.empty())
		{
			return reinterpret_cast<const uint8_t*>(&m_data[0]);
		}
		return nullptr;
	}

	const size_t GetSize() const override
	{
		return sizeof(uint16_t) * m_data.size();
	}

	const IndexBufferFormat GetFormat() const override { return IndexBufferFormat::UInt16; }

private:
	std::vector<uint16_t> m_data;
};


class IndexBufferData32 : public IIndexBufferData
{
public:
	IndexBufferData32(std::initializer_list<uint32_t> initializer) : m_data(initializer) {}

	const uint8_t* GetData() const override
	{
		if (!m_data.empty())
		{
			return reinterpret_cast<const uint8_t*>(&m_data[0]);
		}
		return nullptr;
	}

	const size_t GetSize() const override
	{
		return sizeof(uint32_t) * m_data.size();
	}

	const IndexBufferFormat GetFormat() const override { return IndexBufferFormat::UInt32; }

private:
	std::vector<uint32_t> m_data;
};


} // namespace Kodiak